#pragma once
#include "PictureBuffer.h"
#include "Render/VulkanContext.h"
#include "Vulkan/CommandPool.h"
#include "Vulkan/MemoryAllocator.h"
#include "Vulkan/ResourceAllocator.h"

namespace toystation {

struct EncodeConfig {
    uint32_t codec;
    uint32_t width;
    uint32_t height;
    uint32_t aligned_width;
    uint32_t aligned_height;
    uint32_t luma_plane_size;
    uint32_t chroma_plane_size;
    uint32_t full_image_size;
    uint32_t start_frame;
    uint32_t num_frames;
    uint32_t codec_block_alignment;  // 16 - H264
    uint32_t qp;
    // char inFileName[256];
    // char outFileName[256];
    uint32_t chroma_format_idc;
    VkFormat input_vkformat;
    uint32_t bytepp;  // 1 bytepp = 8 bpp
    uint32_t bpp;
    FILE* input_vid;  // YUV file
    // mio::basic_mmap<mio::access_mode::read, uint8_t> inputVideoMmap;
    FILE* output_vid;  // compressed H264 file
    uint32_t log_batch_encoding : 1;
};

class IntraFrameInfo {
public:
    IntraFrameInfo(uint32_t frame_count, uint32_t width, uint32_t height,
                   StdVideoH264SequenceParameterSet sps,
                   StdVideoH264PictureParameterSet pps, bool is_hdr);
    inline VkVideoEncodeH264VclFrameInfoEXT* GetEncodeH264FrameInfo() {
        return &h264_frame_info_;
    };

private:
    StdVideoEncodeH264SliceHeaderFlags slice_header_flags_ = {};
    StdVideoEncodeH264SliceHeader slice_header_ = {};
    VkVideoEncodeH264NaluSliceInfoEXT slice_info_ = {};
    StdVideoEncodeH264PictureInfoFlags picture_info_flags_ = {};
    StdVideoEncodeH264PictureInfo picture_info_ = {};
    VkVideoEncodeH264VclFrameInfoEXT h264_frame_info_ = {};
};
class VideoSessionParametersInfo {
private:
    VkVideoSessionKHR video_session_;
    VkVideoEncodeH264SessionParametersAddInfoEXT h264_session_par_add_info_;
    VkVideoEncodeH264SessionParametersCreateInfoEXT
        h264_session_par_create_info_;
    VkVideoSessionParametersCreateInfoKHR session_par_create_info_;
};

class EncodeInfo {
public:
    VkVideoEncodeInfoKHR* GetVideoEncodeInfo() { return &encode_info_; };

protected:
    VkVideoEncodeInfoKHR encode_info_;
};

class EncodeInfoNonVcl : public EncodeInfo {
public:
    EncodeInfoNonVcl(StdVideoH264SequenceParameterSet* sps,
                     StdVideoH264PictureParameterSet* pps,
                     VkBuffer* dst_bitstream_buffer)
        : emit_parameters_{} {
        emit_parameters_.sType =
            VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_EMIT_PICTURE_PARAMETERS_INFO_EXT;
        emit_parameters_.pNext = NULL;
        emit_parameters_.spsId = sps->seq_parameter_set_id;
        emit_parameters_.emitSpsEnable = VK_TRUE;
        emit_parameters_.ppsIdEntryCount = 1;
        emit_parameters_.ppsIdEntries = &pps->pic_parameter_set_id;

        memset(&encode_info_, 0, sizeof(encode_info_));
        encode_info_.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR;
        encode_info_.pNext = &emit_parameters_;
        encode_info_.dstBitstreamBuffer = *dst_bitstream_buffer;
    }

private:
    VkVideoEncodeH264EmitPictureParametersInfoEXT emit_parameters_;
};

class EncodeInfoVcl : public EncodeInfo {
public:
    EncodeInfoVcl(VkBuffer* dst_bitstream_buffer,
                  VkDeviceSize dst_bitstream_buffer_offset,
                  VkVideoEncodeH264VclFrameInfoEXT* encode_h264_frame_info,
                  VkVideoPictureResourceInfoKHR* input_pic_resource,
                  VkVideoPictureResourceInfoKHR* dpb_pic_resource)
        : reference_slot_{} {
        reference_slot_.sType = VK_STRUCTURE_TYPE_VIDEO_REFERENCE_SLOT_INFO_KHR;
        reference_slot_.pNext = NULL;
        reference_slot_.slotIndex = 0;
        reference_slot_.pPictureResource = dpb_pic_resource;

        memset(&encode_info_, 0, sizeof(encode_info_));
        encode_info_.sType = VK_STRUCTURE_TYPE_VIDEO_ENCODE_INFO_KHR;
        encode_info_.pNext = encode_h264_frame_info;
        encode_info_.qualityLevel = 0;
        encode_info_.dstBitstreamBuffer = *dst_bitstream_buffer;
        encode_info_.dstBitstreamBufferOffset = dst_bitstream_buffer_offset;
        encode_info_.srcPictureResource = *input_pic_resource;
        encode_info_.pSetupReferenceSlot = &reference_slot_;
    }

private:
    VkVideoReferenceSlotInfoKHR reference_slot_;
};

class VideoSessionParameters {
public:
    StdVideoH264SequenceParameterSet sequence_parameter_set;
    StdVideoH264PictureParameterSet picture_parameter_set;
    VkVideoSessionParametersKHR encode_session_parameters;
};

/// @brief 配置vulkan encode pipeline
class TS_CPP_API EncodeContext {
public:
    void Init(EncodeConfig* config);
    /// @brief 创建codec family的commandpool
    void CreateCommandPool();
    /// @brief 根据编码codec的不同填充video_profile_结构体,当前仅测试一组配置参数
    /// chroma   subsampling 420  8bit
    /// luma     8 bit
    void InitVideoProfile();
    void GetSupportedFormat();
    bool GetVideoFormat(VkImageUsageFlags image_usages,
                        std::vector<VkFormat>& formats);
    void InitDeviceAllocator();

    void CreateSession();
    void InitFramePool();
    void CreateSpsPps();
    void InitRateControl(VkCommandBuffer cmd_buffer, int32_t encode_qp);
    void LoadFrame(EncodeConfig* config,uint32_t framecount,uint32_t current_framebuffer_idx);
    void EncodeFrame(EncodeConfig* config, uint32_t frame_count, bool non_vcl,
                     uint32_t current_framebuffer_idx);
    int32_t BatchSubmit(uint32_t first_framebuffer_idx,
                        uint32_t frames_in_batch);
    int32_t GetEncodedData(EncodeConfig* config, bool non_vcl,
                           uint32_t current_framebuffer_idx);

    void Close();

private:
    VkFormat CodecGetVkFormat(
        VkVideoChromaSubsamplingFlagBitsKHR chrome_format_idc,
        VkVideoComponentBitDepthFlagBitsKHR luma_bit_depth, bool semi_planar);

private:
    VkVideoProfileInfoKHR video_profile_;
    VkVideoEncodeH264ProfileInfoEXT encode_h264_profiles_request_;
    VkVideoEncodeH265ProfileInfoEXT encode_h265_profiles_request_;

    VkVideoSessionKHR video_session_;
    VkVideoSessionParametersKHR encode_session_parameters_;

    VideoSessionParameters video_session_parameters_;

    VkMemoryAllocator mem_alloc_;
    DedicatedResourceAllocator resource_alloc_;

    VkFormat image_format_;

    VkExtent2D max_coded_extent_;
    uint32_t max_reeferent_pictures_slots_count_;

    CommandPool command_pool_;
    PictureBuffer picture_buffer_;
    VkQueue queue_;
    EncodeConfig* current_config_;
};
}  // namespace toystation