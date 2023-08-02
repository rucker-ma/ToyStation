#pragma once
#include <any>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "MetaArg.h"

namespace toystation {
namespace reflect {
class RawMetaBuilder;
}
class Meta {
public:
    [[nodiscard]] const std::string& Name() const { return name_; }

private:
    friend class reflect::RawMetaBuilder;
    std::string name_;
};

class MetaVariable : public Meta {
public:
    MetaVariable() {}
    template <typename T, class Class>
    MetaVariable(T Class::*var) {
        getter_ = [var](std::any obj) -> std::any {
            return std::any_cast<const Class*>(obj)->*var;
        };

        setter_ = [var](std::any obj, std::any value) {
            auto* self = std::any_cast<Class*>(obj);
            self->*var = std::any_cast<T>(value);
        };
    }

    template <typename T, class Class>
    T GetValue(const Class& obj) const {
        return std::any_cast<T>(getter_(&obj));
    }
    template <typename T, class Class>
    void SetValue(Class& obj, T value) {
        setter_(&obj, value);
    }

private:
    friend class reflect::RawMetaBuilder;
    std::function<std::any(std::any)> getter_{nullptr};
    std::function<void(std::any, std::any)> setter_{nullptr};
};

class ArgWrap {
public:
    template <typename T>
    ArgWrap(T&& value) {
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
                throw std::runtime_error(
                    "Cannot cast const ref to non-const ref");
            }

            return *std::any_cast<RawT*>(storage_);
        }
    }

private:
    bool is_ref_{false};
    bool is_const_{false};

    std::any storage_{};
};

template <typename... Args, size_t N, size_t... Is>
std::tuple<Args...> AsTuple(std::array<ArgWrap, N>& array,
                            std::index_sequence<Is...>) {
    return std::forward_as_tuple(array[Is].template Cast<Args>()...);
}

template <typename... Args, size_t N,
          typename = std::enable_if_t<N == sizeof...(Args)>>
std::tuple<Args...> AsTuple(std::array<ArgWrap, N>& array) {
    return AsTuple<Args...>(array, std::make_index_sequence<N>());
}

class MetaFunction : public Meta {
public:
    MetaFunction() {}
    template <class Class, typename... Args>
    explicit MetaFunction(void (Class::*func)(Args...)) {
        args_num_ = sizeof...(Args);

        function_ = [this, func](void* args_ptr) -> std::any {
            auto& args =
                *static_cast<std::array<ArgWrap, sizeof...(Args) + 1>*>(
                    args_ptr);
            auto tuple = AsTuple<Class&, Args...>(args);

            std::apply(func, tuple);
            return std::any{};
        };
    }

    template <class Class, typename... Args>
    explicit MetaFunction(void (Class::*func)(Args...) const) {
        args_num_ = sizeof...(Args);
        function_ = [this, func](void* args_ptr) -> std::any {
            auto& args =
                *static_cast<std::array<ArgWrap, sizeof...(Args) + 1>*>(
                    args_ptr);
            auto tuple = AsTuple<Class&, Args...>(args);

            std::apply(func, tuple);
            return std::any{};
        };
    }

    template <class Class, typename Return, typename... Args>
    explicit MetaFunction(Return (Class::*func)(Args...)) {
        args_num_ = sizeof...(Args);
        function_ = [this, func](void* args_ptr) -> std::any {
            auto& args =
                *static_cast<std::array<ArgWrap, sizeof...(Args) + 1>*>(
                    args_ptr);
            auto tuple = AsTuple<Class&, Args...>(args);

            return std::apply(func, tuple);
        };
    }

    template <class Class, typename Return, typename... Args>
    explicit MetaFunction(Return (Class::*func)(Args...) const) {
        args_num_ = sizeof...(Args);
        function_ = [this, func](void* args_ptr) -> std::any {
            auto& args =
                *static_cast<std::array<ArgWrap, sizeof...(Args) + 1>*>(
                    args_ptr);
            auto tuple = AsTuple<Class&, Args...>(args);

            return std::apply(func, tuple);
        };
    }

    template <class Class, typename... Args>
    std::any Invoke(Class& obj, Args&&... args) {
        if (args_num_ != sizeof...(Args)) {
            throw std::runtime_error("Mismatching number of arguments");
        }

        std::array<ArgWrap, sizeof...(Args) + 1> args_array{
            ArgWrap(obj), ArgWrap(std::forward<Args>(args))...};

        return function_(&args_array);
    }
private:
    friend class reflect::RawMetaBuilder;
    std::function<std::any(void*)> function_;
    int args_num_;
};

class MetaClass : public Meta {
public:
    MetaClass() {}
    [[nodiscard]] const std::vector<MetaVariable>& GetVariables() const {
        return variables_;
    }

    [[nodiscard]] const std::vector<MetaFunction>& GetFunctions() const {
        return functions_;
    }

    MetaVariable GetVariable(const std::string& name) const {
        for (auto& var : variables_) {
            if (var.Name() == name) {
                return var;
            }
        }
        return MetaVariable{};
    }

    MetaFunction GetFunction(const std::string& name) const {
        for (auto& func : functions_) {
            if (func.Name() == name) {
                return func;
            }
        }
        return MetaFunction{};
    }

private:
    friend class reflect::RawMetaBuilder;

    std::vector<MetaVariable> variables_;
    std::vector<MetaFunction> functions_;
};

class Registry {
public:
    static Registry& Instance() {
        static Registry instance;
        return instance;
    }
    MetaClass* Find(const std::string& name) {
        return metas_.find(name)->second.get();
    }
    void Register(std::unique_ptr<MetaClass> desc) {
        metas_[desc->Name()] = std::move(desc);
    }

private:
    std::unordered_map<std::string, std::unique_ptr<MetaClass>> metas_;
};

namespace reflect {

class RawMetaBuilder {
public:
    explicit RawMetaBuilder(const std::string& name)
        : desc_(std::make_unique<MetaClass>()) {
        desc_->name_ = name;
    }
    RawMetaBuilder(const RawMetaBuilder&) = delete;
    ~RawMetaBuilder() { Registry::Instance().Register(std::move(desc_)); }

    template <class Class, typename Var>
    void AddVariable(const std::string& name, Var Class::*var) {
        MetaVariable variable(var);
        variable.name_ = name;
        desc_->variables_.emplace_back(std::move(variable));
    }

    template <class Class, typename Func>
    void AddFunction(const std::string& name, Func Class::*func) {
        MetaFunction function(func);
        function.name_ = name;
        desc_->functions_.emplace_back(std::move(function));
    }

private:
    std::unique_ptr<MetaClass> desc_{nullptr};
};

template <class Class>
class MetaBuilder {
public:
    explicit MetaBuilder(const std::string& name) : raw_builder_(name) {}

    template <typename Var>
    MetaBuilder& AddVariable(const std::string& name, Var Class::*var) {
        raw_builder_.AddVariable(name, var);
        return *this;
    }

    template <typename Func>
    MetaBuilder& AddFunction(const std::string& name, Func Class::*func) {
        raw_builder_.AddFunction(name, func);
        return *this;
    }

private:
    RawMetaBuilder raw_builder_;
};

template <class Class>
MetaBuilder<Class> AddClass(const std::string& name) {
    return MetaBuilder<Class>(name);
}
MetaClass& GetByName(const std::string& name);

}  // namespace reflect

#define META_CLASS(name) \
  namespace toystation { \
  class name##Builder {  \
   public:               \
    name##Builder();     \
  };                     \
  }

#define GENERATE_BODY(name)                                               \
  friend class name##Builder;                                             \
                                                                          \
 public:                                                                  \
  static MetaClass& Class() { return *Registry::Instance().Find(#name); } \
                                                                          \
 private:                                                                 \
  static name##Builder builder_;

#define BEGIN_DEFINE(name)      \
  namespace toystation { \
  name##Builder name::builder_; \
  name##Builder::name##Builder() {\
  reflect::AddClass<name>(#name)
#define END_DEFINE(...) ;}}

#define SKIP_GENERATE(...) __VA_ARGS__

}  // namespace toystation