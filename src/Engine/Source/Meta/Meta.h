#pragma once
#include <any>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <cassert>


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
class MetaClass;
namespace reflect {
MetaClass& GetByName(const std::string& name);

template <typename T>
MetaClass* GetByType() {
//    static MetaClass null_meta;
    return nullptr;
}
}  // namespace reflect


namespace reflect {

class MetaArg {
public:
    MetaArg():type_ptr_(nullptr){}

    MetaArg(const MetaArg& arg){
        _copy(arg);
    }
    MetaArg(MetaArg&& arg){
        //_copy(arg);
        _move(std::forward<MetaArg>(arg));
    }
    MetaArg& operator=(MetaArg&& arg){
        //_copy(arg);
        _move(std::forward<MetaArg>(arg));
        return *this;
    }
    MetaArg& operator=(const MetaArg& arg){
        if(&arg == this){
            return *this;
        }
        _copy(arg);
        return *this;
    }
    bool IsConst()const{return is_const_;}
    bool IsRef()const{return is_ref_;}
    bool IsEmpty()const{return storage_.has_value();}
    template <class T>
    MetaArg(std::shared_ptr<T>&& value_ptr) {
        _init(std::move(value_ptr.get()));
        using RawT = std::remove_pointer_t<std::remove_cvref_t<T>>;
        _get_meta<RawT>();
    }

    //通过is_same判断避免MetaArg被重复包装
    template <typename T,std::enable_if_t<std::negation_v<std::is_same<std::remove_cvref_t<T>, MetaArg>>,int> = 0 >
    MetaArg(T&& value) {
        //对于数值类型统一按double存储
        if constexpr (std::is_arithmetic_v<T>) {
            double number =value;
            storage_ = number;
            type_ptr_ = nullptr;
            return;
        }
        _init(std::forward<T>(value));
        using RawT = std::remove_pointer_t<std::remove_cvref_t<T>>;
        _get_meta<RawT>();

    }
    MetaClass* Meta() {
        return type_ptr_;
    }

    template <typename T>
    T Cast() {
        using RawT = std::remove_cvref_t<T>;
        constexpr bool cast_ref = std::is_reference_v<T>;
        constexpr bool cast_const = std::is_const_v<T>;
        constexpr bool cast_pointer = std::is_pointer_v<T>;
        if constexpr (std::is_arithmetic_v<T>){
            double number = std::any_cast<double>(storage_);
            return static_cast<T>(number);
        }

        if constexpr (!cast_ref) {
            if (is_ref_) {
                // 引用类型转换为值类型
                if (is_const_) {
                    return *std::any_cast<const RawT*>(storage_);
                }
                else {
                 if(cast_pointer) {
                     return std::any_cast<RawT>(storage_);
                 }else{
                     return *std::any_cast<RawT*>(storage_);
                 }
                }
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
                assert(0&&"Cannot cast const ref to non-const ref");
                //throw std::runtime_error("Cannot cast const ref to non-const ref");
            }
            return *std::any_cast<RawT*>(storage_);
        }
    }
private:
    void _copy(const MetaArg& arg){
        this->storage_ = arg.storage_;
        this->is_ref_ = arg.is_ref_;
        this->is_const_ = arg.is_const_;
        this->type_ptr_ = arg.type_ptr_;
    }
    void _move(MetaArg&& arg) {
        this->storage_ = std::move(arg.storage_);
        this->is_ref_ = arg.is_ref_;
        this->is_const_ = arg.is_const_;
        this->type_ptr_ = arg.type_ptr_;
    }
    //右值引用，对于基础数值类型
    // 对于指针类型
    template <class T>
    void _init(T&& value){
        using RawType = std::remove_pointer_t<T>;
//        is_ref_ = std::is_reference_v<RawType>;
        is_ref_ = false;
        is_const_ = std::is_const_v<RawType>;
        if (is_ref_) {
            storage_ = &value;
        } else {
            storage_ = value;
        }
    }
    //左值引用类型
    template <class T>
    void _init(T& value){
        using RawType = std::remove_pointer_t<T>;

//        is_ref_ = std::is_reference_v<RawType>;
        is_ref_ = true;
        is_const_ = std::is_const_v<RawType>;

        if (is_ref_) {
            storage_ = &value;
        } else {
            storage_ = value;
        }
    }

    template <class T>
    void _get_meta(){
        type_ptr_ = GetByType<T>();
    }
private:
    bool is_ref_{false};
    bool is_const_{false};
    MetaClass* type_ptr_;
    std::any storage_{};
};

}

class MetaVariable : public Meta {
public:
    MetaVariable() {}
    template <typename T, class Class,
              std::enable_if_t<std::copyable<T>, int> = 0>
    MetaVariable(T Class::*var) {
        getter_ = [var](reflect::MetaArg obj) -> reflect::MetaArg {
            if (obj.IsConst()) {
                auto* self = obj.Cast<const Class*>();
                return self->*var;
            } else {
                auto* self = obj.Cast<Class*>();
                return self->*var;
            }
        };
        setter_ = [var](reflect::MetaArg obj, reflect::MetaArg value) {
            // auto* self = std::any_cast<Class*>(obj);
            // self->*var = std::any_cast<T>(value);
            auto* self = obj.Cast<Class*>();
            self->*var = value.Cast<T>();
        };
    }

    template <typename T, class Class>
    T GetValue(const Class& obj) const {
        // Class* to MetaArg
        return getter_(&obj).template Cast<T>();
    }
    template <typename T, class Class>
    void SetValue(Class& obj, T value) {
        setter_(&obj, value);
    }

    reflect::MetaArg GetValue(reflect::MetaArg obj) const {
        return getter_(obj);
    }
    void SetValue(reflect::MetaArg obj, reflect::MetaArg value) {
        setter_(obj, value);
    }

private:
    friend class reflect::RawMetaBuilder;
    std::function<reflect::MetaArg(reflect::MetaArg)> getter_{nullptr};
    std::function<void(reflect::MetaArg, reflect::MetaArg)> setter_{nullptr};
};

template <typename... Args, size_t N, size_t... Is>
std::tuple<Args...> AsTuple(std::array<reflect::MetaArg, N>& array,
                            std::index_sequence<Is...>) {
    return std::forward_as_tuple(array[Is].template Cast<Args>()...);
}

template <typename... Args, size_t N,
          typename = std::enable_if_t<N == sizeof...(Args)>>
std::tuple<Args...> AsTuple(std::array<reflect::MetaArg, N>& array) {
    return AsTuple<Args...>(array, std::make_index_sequence<N>());
}

using MetaConstrcut =  std::function<reflect::MetaArg(std::vector<reflect::MetaArg> args)>;

template<class T,class ArgTuple,size_t... Indices>
T* NewObject(ArgTuple tpl,std::index_sequence<Indices...>){
    return new T(std::get<Indices>(std::forward<ArgTuple>(tpl))...);
}

template<class Class,typename... Args>
MetaConstrcut MakeConstruct(){
    int args_num = sizeof...(Args);
    auto function = [args_num](std::vector<reflect::MetaArg> args) -> reflect::MetaArg {
        assert(args.size() == args_num);
        std::array<reflect::MetaArg, sizeof...(Args)> consctruct_args;
        for (size_t i = 0; i < sizeof...(Args); i++) {
            consctruct_args[i] = args[i];
        }
        auto tuple = AsTuple<Args...>(consctruct_args);
        auto* obj = NewObject<Class>(tuple,
                                     std::make_index_sequence<sizeof...(Args)>{});
        return reflect::MetaArg(std::move(obj));
    };
    return function;
}

class MetaFunction : public Meta {
public:
    MetaFunction() {}
    template <class Class, typename... Args>
    explicit MetaFunction(void (Class::*func)(Args...)) {
        args_num_ = sizeof...(Args);

        function_ =
            [this,
             func](std::vector<reflect::MetaArg>& args) -> reflect::MetaArg {
            std::array<reflect::MetaArg, sizeof...(Args) + 1> apply_args;
            for (size_t i = 0; i < sizeof...(Args) + 1; i++) {
                apply_args[i] = args[i];
            }
            auto tuple = AsTuple<Class*, Args...>(apply_args);
            std::apply(func, tuple);
            return reflect::MetaArg();
        };
    }

    template <class Class, typename... Args>
    explicit MetaFunction(void (Class::*func)(Args...) const) {
        args_num_ = sizeof...(Args);
        function_ =
            [this,
             func](std::vector<reflect::MetaArg>& args) -> reflect::MetaArg {
            std::array<reflect::MetaArg, sizeof...(Args) + 1> apply_args;
            for (size_t i = 0; i < sizeof...(Args) + 1; i++) {
                apply_args[i] = args[i];
            }
            auto tuple = AsTuple<Class*, Args...>(apply_args);
            std::apply(func, tuple);
            return reflect::MetaArg();
        };
    }

    template <class Class, typename Return, typename... Args>
    explicit MetaFunction(Return (Class::*func)(Args...)) {
        args_num_ = sizeof...(Args);
        function_ =
            [this,
             func](std::vector<reflect::MetaArg>& args) -> reflect::MetaArg {
            std::array<reflect::MetaArg, sizeof...(Args) + 1> apply_args;
            for (size_t i = 0; i < sizeof...(Args) + 1; i++) {
                apply_args[i] = args[i];
            }
            auto tuple = AsTuple<Class*, Args...>(apply_args);
            return std::apply(func, tuple);
        };
    }

    template <class Class, typename Return, typename... Args>
    explicit MetaFunction(Return (Class::*func)(Args...) const) {
        args_num_ = sizeof...(Args);
        function_ =
            [this,
             func](std::vector<reflect::MetaArg>& args) -> reflect::MetaArg {
            std::array<reflect::MetaArg, sizeof...(Args) + 1> apply_args;
            for (size_t i = 0; i < sizeof...(Args) + 1; i++) {
                apply_args[i] = args[i];
            }
            auto tuple = AsTuple<Class*, Args...>(apply_args);
            return std::apply(func, tuple);
        };
    }

    reflect::MetaArg Invoke(std::vector<reflect::MetaArg> args) {
        if (args_num_ != args.size() - 1) {
            assert(0 && "Mismatching number of arguments");
            // throw std::runtime_error("Mismatching number of arguments");
        }
        return function_(args);
    }
    int ArgNum(){return args_num_;}
private:
    friend class reflect::RawMetaBuilder;
    std::function<reflect::MetaArg(std::vector<reflect::MetaArg>& args)>
        function_;
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
    MetaConstrcut GetConstruct(int arg_num)const {
        if(constrcuts_.find(arg_num)!=constrcuts_.end()) {
            return constrcuts_.at(arg_num);
        }
       return {};
    }
    const MetaFunction* GetFunctionPtr(const std::string& name) const {
        for (auto iter = functions_.begin(); iter < functions_.end(); iter++) {
            if (iter->Name() == name) {
                return &(*iter);
            }
        }
        return nullptr;
    }

private:
    friend class reflect::RawMetaBuilder;
    //TODO:map vs vector?
    std::vector<MetaVariable> variables_;
    std::vector<MetaFunction> functions_;
    //TODO:多个构造函数如何区分？
    std::map<int,MetaConstrcut> constrcuts_;
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
    template <class Class,typename... Types>
    void AddConstruct(){
        auto func = MakeConstruct<Class,Types...>();
        int arg_num = sizeof...(Types);
        desc_->constrcuts_.insert(std::pair(arg_num,func));
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
    template<typename... Args>
    MetaBuilder& AddConstruct() {
        raw_builder_.AddConstruct<Class,Args...>();
        return *this;
    }
private:
    RawMetaBuilder raw_builder_;
};

template <class Class>
MetaBuilder<Class> AddClass(const std::string& name) {
    return MetaBuilder<Class>(name);
}
}  // namespace reflect

#define META_CLASS(name)           \
    namespace toystation {         \
    class name;                    \
    namespace reflect {            \
    template <>                    \
    inline MetaClass* GetByType<name>() { \
        return &(GetByName(#name));\
    }                              \
    }                              \
    class name##Builder {          \
    public:                        \
        name##Builder();           \
    };                             \
    }

#define GENERATE_BODY(name)                                                 \
    friend class name##Builder;                                             \
                                                                            \
public:                                                                     \
    static MetaClass& Class() { return *Registry::Instance().Find(#name); } \
                                                                            \
private:                                                                    \
    static name##Builder builder_;

#define BEGIN_DEFINE(name)           \
    namespace toystation {           \
    name##Builder name::builder_;    \
    name##Builder::name##Builder() { \
        reflect::AddClass<name>(#name)
#define END_DEFINE(...) \
    ;                   \
    }                   \
    }

#define SKIP_GENERATE(...) __VA_ARGS__

}  // namespace toystation