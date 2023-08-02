//
// Created by ma on 2023/8/2.
//

#pragma once
#include <any>
#include <exception>

namespace toystation {
namespace reflect {

class MetaArg {
public:
    template <typename T>
    MetaArg(T&& value) {
        is_ref_ = std::is_reference_v<T>;
        is_const_ = std::is_const_v<T>;
        if (is_ref_) {
            storage_ = &value;
        } else {
            storage_ = value;
        }
    }

    template <typename T>
    T Cast() {
        using RawT = std::remove_cv_t<std::remove_reference_t<T>>;

        constexpr bool cast_ref = std::is_reference_v<T>;
        constexpr bool cast_const = std::is_const_v<T>;

        if constexpr (!cast_ref) {
            if (is_ref_) {
                // 引用类型转换为值类型
                if (is_const_)
                    return *std::any_cast<const RawT*>(storage_);
                else
                    return *std::any_cast<RawT*>(storage_);
            }
            // 值类型转换为值类型
            return std::any_cast<RawT>(storage_);
        }

        // 值类型转换为引用类型
        if (!is_ref_) {
            return *std::any_cast<RawT>(&storage_);
        }

        // 引用类型转换为引用类型
        if constexpr (cast_const) {
            if (is_const_)
                return *std::any_cast<const RawT*>(storage_);
            else
                return *std::any_cast<RawT*>(storage_);
        } else {
            if (is_const_) {
                // 无法将常量引用转换为非常量引用
                throw std::runtime_error("Cannot cast const ref to non-const ref");
            }

            return *std::any_cast<RawT*>(storage_);
        }
    }

private:
    bool is_ref_{false};
    bool is_const_{false};

    std::any storage_{};
};

}
}