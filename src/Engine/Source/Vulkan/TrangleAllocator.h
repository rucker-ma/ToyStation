#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <algorithm>

namespace toystation {
// GRANULARITY must be power of two
template <uint32_t GRANULARITY = 256>
class TRangeAllocator {
private:
    uint32_t size_;
    uint32_t used_;

public:
    TRangeAllocator() : size_(0), used_(0) {}
    TRangeAllocator(uint32_t size) { Init(size); }

    ~TRangeAllocator() { DeInit(); }

    static uint32_t AlignedSize(uint32_t size) {
        return (size + GRANULARITY - 1) & (~(GRANULARITY - 1));
    }

    void Init(uint32_t size) {
        assert(size % GRANULARITY == 0 &&
               "managed total size must be aligned to GRANULARITY");

        uint32_t pages = ((size + GRANULARITY - 1) / GRANULARITY);
        RangeInit(pages - 1);
        used_ = 0;
        size_ = size;
    }
    void DeInit() { RangeDeInit(); }

    bool IsEmpty() const { return used_ == 0; }

    bool IsAvailable(uint32_t size, uint32_t align) const {
        uint32_t alignRest = align - 1;
        uint32_t sizeReserved = size;

        if (used_ >= size_) {
            return false;
        }

        if (used_ != 0 && align > GRANULARITY) {
            sizeReserved += alignRest;
        }

        uint32_t countReserved = (sizeReserved + GRANULARITY - 1) / GRANULARITY;
        return IsRangeAvailable(countReserved);
    }

    bool SubAllocate(uint32_t size, uint32_t align, uint32_t& out_offset,
                     uint32_t& out_aligned, uint32_t& out_size) {
        if (align == 0) {
            align = 1;
        }
        uint32_t alignRest = align - 1;
        uint32_t sizeReserved = size;
//#if (defined(NV_X86) || defined(NV_X64)) && defined(_MSC_VER)
        bool alignIsPOT = __popcnt(align) == 1;
// #else
//         bool alignIsPOT = __builtin_popcount(align) == 1;
// #endif

        if (used_ >= size_) {
            out_size = 0;
            out_offset = 0;
            out_aligned = 0;
            return false;
        }

        if (used_ != 0 && (alignIsPOT ? (align > GRANULARITY)
                                      : ((alignRest + size) > GRANULARITY))) {
            sizeReserved += alignRest;
        }

        uint32_t countReserved = (sizeReserved + GRANULARITY - 1) / GRANULARITY;

        uint32_t startID;
        if (CreateRangeID(startID, countReserved)) {
            out_offset = startID * GRANULARITY;
            out_aligned = ((out_offset + alignRest) / align) * align;

            // due to custom alignment, we may be able to give
            // pages back that we over-allocated
            //
            // reserved:   [     |     |     |     ] (GRANULARITY spacing)
            // used:                [      ]         (custom alignment/size)
            // corrected:        [     |     ]       (GRANULARITY spacing)

            // correct start (warning could yield more fragmentation)

            uint32_t skipFront = (out_aligned - out_offset) / GRANULARITY;
            if (skipFront) {
                DestroyRangeID(startID, skipFront);
                out_offset += skipFront * GRANULARITY;
                startID += skipFront;
                countReserved -= skipFront;
            }

            assert(out_offset <= out_aligned);

            // correct end
            uint32_t outLast = AlignedSize(out_aligned + size);
            out_size = outLast - out_offset;

            uint32_t usedCount = out_size / GRANULARITY;
            assert(usedCount <= countReserved);

            if (usedCount < countReserved) {
                DestroyRangeID(startID + usedCount, countReserved - usedCount);
            }

            assert((out_aligned + size) <= (out_offset + out_size));

            used_ += out_size;

            // checkRanges();

            return true;
        } else {
            out_size = 0;
            out_offset = 0;
            out_aligned = 0;
            return false;
        }
    }

    void SubFree(uint32_t offset, uint32_t size) {
        assert(offset % GRANULARITY == 0);
        assert(size % GRANULARITY == 0);

        used_ -= size;
        DestroyRangeID(offset / GRANULARITY, size / GRANULARITY);

        // checkRanges();
    }

    TRangeAllocator& operator=(const TRangeAllocator& other) {
        size_ = other.size_;
        used_ = other.used_;

        ranges_ = other.ranges_;
        count_ = other.count_;
        capacity_ = other.capacity_;
        max_id_ = other.max_id_;

        if (ranges_) {
            ranges_ = static_cast<Range*>(::malloc(capacity_ * sizeof(Range)));
            memcpy(ranges_, other.ranges_, capacity_ * sizeof(Range));
        }

        return *this;
    }

    TRangeAllocator(const TRangeAllocator& other) {
        size_ = other.size_;
        used_ = other.used_;

        ranges_ = other.ranges_;
        count_ = other.count_;
        capacity_ = other.capacity_;
        max_id_ = other.max_id_;

        if (ranges_) {
            ranges_ = static_cast<Range*>(::malloc(capacity_ * sizeof(Range)));
            assert(ranges_);  // Make sure allocation succeeded
            memcpy(ranges_, other.ranges_, capacity_ * sizeof(Range));
        }
    }

    TRangeAllocator& operator=(TRangeAllocator&& other) {
        size_ = other.size_;
        used_ = other.used_;

        ranges_ = other.ranges_;
        count_ = other.count_;
        capacity_ = other.capacity_;
        max_id_ = other.max_id_;

        other.ranges_ = nullptr;

        return *this;
    }

    TRangeAllocator(TRangeAllocator&& other) {
        size_ = other.size_;
        used_ = other.used_;

        ranges_ = other.ranges_;
        count_ = other.count_;
        capacity_ = other.capacity_;
        max_id_ = other.max_id_;

        other.ranges_ = nullptr;
    }

private:
    //////////////////////////////////////////////////////////////////////////
    // most of the following code is taken from Emil Persson's MakeID
    // http://www.humus.name/3D/MakeID.h (v1.02)

    struct Range {
        uint32_t m_First;
        uint32_t m_Last;
    };

    Range* ranges_ = nullptr;  // Sorted array of ranges of free IDs
    uint32_t count_ = 0;       // Number of ranges in list
    uint32_t capacity_ = 0;    // Total capacity of range list
    uint32_t max_id_ = 0;

public:
    void RangeInit(const uint32_t max_id) {
        // Start with a single range, from 0 to max allowed ID (specified)
        ranges_ = static_cast<Range*>(::malloc(sizeof(Range)));
        assert(ranges_ != nullptr);  // Make sure allocation succeeded
        ranges_[0].m_First = 0;
        ranges_[0].m_Last = max_id;
        count_ = 1;
        capacity_ = 1;
        max_id_ = max_id;
    }

    void RangeDeInit() {
        if (ranges_) {
            ::free(ranges_);
            ranges_ = nullptr;
        }
    }

    bool CreateID(uint32_t& id) {
        if (ranges_[0].m_First <= ranges_[0].m_Last) {
            id = ranges_[0].m_First;

            // If current range is full and there is another one, that will
            // become the new current range
            if (ranges_[0].m_First == ranges_[0].m_Last && count_ > 1) {
                destroyRange(0);
            } else {
                ++ranges_[0].m_First;
            }
            return true;
        }

        // No availble ID left
        return false;
    }

    bool CreateRangeID(uint32_t& id, const uint32_t count) {
        uint32_t i = 0;
        do {
            const uint32_t range_count =
                1 + ranges_[i].m_Last - ranges_[i].m_First;
            if (count <= range_count) {
                id = ranges_[i].m_First;

                // If current range is full and there is another one, that will
                // become the new current range
                if (count == range_count && i + 1 < count_) {
                    DestroyRange(i);
                } else {
                    ranges_[i].m_First += count;
                }
                return true;
            }
            ++i;
        } while (i < count_);

        // No range of free IDs was large enough to create the requested
        // continuous ID sequence
        return false;
    }

    bool DestroyID(const uint32_t id) { return DestroyRangeID(id, 1); }

    bool DestroyRangeID(const uint32_t id, const uint32_t count) {
        const uint32_t end_id = id + count;

        assert(end_id <= max_id_ + 1);

        // Binary search of the range list
        uint32_t i0 = 0;
        uint32_t i1 = count_ - 1;

        for (;;) {
            const uint32_t i = (i0 + i1) / 2;

            if (id < ranges_[i].m_First) {
                // Before current range, check if neighboring
                if (end_id >= ranges_[i].m_First) {
                    if (end_id != ranges_[i].m_First)
                        return false;  // Overlaps a range of free IDs, thus (at
                                       // least partially) invalid IDs

                    // Neighbor id, check if neighboring previous range too
                    if (i > i0 && id - 1 == ranges_[i - 1].m_Last) {
                        // Merge with previous range
                        ranges_[i - 1].m_Last = ranges_[i].m_Last;
                        DestroyRange(i);
                    } else {
                        // Just grow range
                        ranges_[i].m_First = id;
                    }
                    return true;
                } else {
                    // Non-neighbor id
                    if (i != i0) {
                        // Cull upper half of list
                        i1 = i - 1;
                    } else {
                        // Found our position in the list, insert the deleted
                        // range here
                        InsertRange(i);
                        ranges_[i].m_First = id;
                        ranges_[i].m_Last = end_id - 1;
                        return true;
                    }
                }
            } else if (id > ranges_[i].m_Last) {
                // After current range, check if neighboring
                if (id - 1 == ranges_[i].m_Last) {
                    // Neighbor id, check if neighboring next range too
                    if (i < i1 && end_id == ranges_[i + 1].m_First) {
                        // Merge with next range
                        ranges_[i].m_Last = ranges_[i + 1].m_Last;
                        DestroyRange(i + 1);
                    } else {
                        // Just grow range
                        ranges_[i].m_Last += count;
                    }
                    return true;
                } else {
                    // Non-neighbor id
                    if (i != i1) {
                        // Cull bottom half of list
                        i0 = i + 1;
                    } else {
                        // Found our position in the list, insert the deleted
                        // range here
                        InsertRange(i + 1);
                        ranges_[i + 1].m_First = id;
                        ranges_[i + 1].m_Last = end_id - 1;
                        return true;
                    }
                }
            } else {
                // Inside a free block, not a valid ID
                return false;
            }
        }
    }

    bool IsRangeAvailable(uint32_t search_count) const {
        uint32_t i = 0;
        do {
            uint32_t count = ranges_[i].m_Last - ranges_[i].m_First + 1;
            if (count >= search_count) return true;

            ++i;
        } while (i < count_);

        return false;
    }

    void PrintRanges() const {
        uint32_t i = 0;
        for (;;) {
            if (ranges_[i].m_First < ranges_[i].m_Last)
                printf("%u-%u", ranges_[i].m_First, ranges_[i].m_Last);
            else if (ranges_[i].m_First == ranges_[i].m_Last)
                printf("%u", ranges_[i].m_First);
            else
                printf("-");

            ++i;
            if (i >= count_) {
                printf("\n");
                return;
            }

            printf(", ");
        }
    }

    void CheckRanges() const {
        for (uint32_t i = 0; i < count_; i++) {
            assert(ranges_[i].m_Last <= max_id_);

            if (ranges_[i].m_First == ranges_[i].m_Last + 1) {
                continue;
            }
            assert(ranges_[i].m_First <= ranges_[i].m_Last);
            assert(ranges_[i].m_First <= max_id_);
        }
    }

    void InsertRange(const uint32_t index) {
        if (count_ >= capacity_) {
            capacity_ += capacity_;
            ranges_ = (Range*)realloc(ranges_, capacity_ * sizeof(Range));
            assert(ranges_);  // Make sure reallocation succeeded
        }

        ::memmove(ranges_ + index + 1, ranges_ + index,
                  (count_ - index) * sizeof(Range));
        ++count_;
    }

    void DestroyRange(const uint32_t index) {
        --count_;
        ::memmove(ranges_ + index, ranges_ + index + 1,
                  (count_ - index) * sizeof(Range));
    }
};

}  // namespace toystation