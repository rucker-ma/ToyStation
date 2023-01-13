#pragma once
#include <tuple>

namespace toystation {

template <std::size_t...>
struct TupleIndices {};


//参数包解包实例
template <std::size_t Sp, class IntTuple, std::size_t Ep>
struct MakeIndicesImpl;
template <std::size_t Sp, std::size_t... Indices, std::size_t Ep>
struct MakeIndicesImpl<Sp, TupleIndices<Indices...>, Ep> {
    typedef
        typename MakeIndicesImpl<Sp + 1, TupleIndices<Indices..., Sp>, Ep>::type
            type;
};
template <std::size_t Ep, std::size_t... Indices>
struct MakeIndicesImpl<Ep, TupleIndices<Indices...>, Ep> {
    typedef TupleIndices<Indices...> type;
};
template <std::size_t Ep, std::size_t Sp = 0>
struct MakeTupleIndices {
    using type = typename MakeIndicesImpl<Sp, TupleIndices<>, Ep>::type;
};

template <class Fp, class... Args>
inline auto invoke(Fp&& func, Args&&... args)
    -> decltype(std::forward<Fp>(func)(std::forward<Args>(args)...)) {
    return std::forward<Fp>(func)(std::forward<Args>(args)...);
}

template <typename F, class... ArgTypes>
class TDelegate {
public:
    TDelegate(const TDelegate&) = delete;
    void operator=(const TDelegate&) = delete;

    void Bind(F&& func, ArgTypes&&... args) {
        tp_ = std::make_tuple(std::forward<F>(func),
                              std::forward<ArgTypes>(args)...);
    }

    template <std::size_t... Indices>
    void Exec() {
        typedef typename MakeTupleIndices<
            std::tuple_size<std::tuple<F, ArgTypes...>>::value, 1>::type
            index_type;
        Exec2(index_type());
    }

private:
    template <std::size_t... Indices>
    void Exec2(TupleIndices<Indices...>) {
        invoke(std::move(std::get<0>(tp_)), std::move(std::get<Indices>(tp_))...);
    }

    std::tuple<typename std::decay_t<F>, typename std::decay_t<ArgTypes>...>
        tp_;
};

}  // namespace toystation