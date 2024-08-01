#pragma once

namespace std
{
    template <size_t... Ints>
    struct integer_sequence
    {
        using type = integer_sequence;
        using value_type = size_t;
        static constexpr std::size_t size() noexcept { return sizeof...(Ints); }
    };

    template <class Sequence1, class Sequence2>
    struct _merge_and_renumber;

    template <size_t... I1, size_t... I2>
    struct _merge_and_renumber<integer_sequence<I1...>, integer_sequence<I2...>>
      : integer_sequence<I1..., (sizeof...(I1)+I2)...>
    { };

    template <size_t N>
    struct make_integer_sequence
      : _merge_and_renumber<typename make_integer_sequence<N/2>::type,
                            typename make_integer_sequence<N - N/2>::type>
    { };

    template<> struct make_integer_sequence<0> : integer_sequence<> { };
    template<> struct make_integer_sequence<1> : integer_sequence<0> { };
}

struct swallow { template<class... T> swallow(T&&...){} };

template<class F>
struct function_traits;

template<class R, class... Args>
struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)> {};

template<class R, class... Args>
struct function_traits<R(Args...)>
{
    using function_type = R(Args...);
    using free_function_type = R(Args...);

    using return_type = R;

    static constexpr std::size_t arity = sizeof...(Args);

    template<std::size_t N>
    struct argument
    {
        static_assert((N < arity), "error: invalid parameter index.");
        using type = typename std::tuple_element<N,std::tuple<Args...>>::type;
    };
};

template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...)> : public function_traits<R(C&,Args...)>
{
    using free_function_type = R(Args...);
};

template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...) const> : public function_traits<R(C&,Args...)>
{
    using free_function_type = R(Args...);
};

template<class C, class R>
struct function_traits<R(C::*)> : public function_traits<R(C&)>
{};

template<int I> struct placeholder{};

namespace std {
    template<int I> struct is_placeholder< ::placeholder<I>> : std::integral_constant<int, I>{};
}

namespace detail {
    template<std::size_t... Is, class F, class... Args>
    auto easy_bind(std::integer_sequence<Is...>, F const& f, Args&&... args)
        -> decltype(std::bind(f, std::forward<Args>(args)..., placeholder<Is + 1>{}...))
    {
        return std::bind(f, std::forward<Args>(args)..., placeholder<Is + 1>{}...);
    }
}

template<class R, class... FArgs, class... Args>
auto easy_bind(std::function<R(FArgs...)> f, Args&&... args)
    -> decltype(detail::easy_bind(std::make_integer_sequence<sizeof...(FArgs) - sizeof...(Args)>{}, f, std::forward<Args>(args)...))
{
    return detail::easy_bind(std::make_integer_sequence<sizeof...(FArgs) - sizeof...(Args)>{}, f, std::forward<Args>(args)...);
}

template<class R, class... FArgs, class... Args>
auto easy_bind(R(*f)(FArgs...), Args&&... args)
    -> decltype(detail::easy_bind(std::make_integer_sequence<sizeof...(FArgs) - sizeof...(Args)>{}, f, std::forward<Args>(args)...))
{
    return detail::easy_bind(std::make_integer_sequence<sizeof...(FArgs) - sizeof...(Args)>{}, f, std::forward<Args>(args)...);
}

template<class R, typename T, class... FArgs, class... Args>
auto easy_bind(R(T::*f)(FArgs...), Args&&... args)
    -> decltype(detail::easy_bind(std::make_integer_sequence<sizeof...(FArgs) + 1 - sizeof...(Args)>{}, f, std::forward<Args>(args)...))
{
    return detail::easy_bind(std::make_integer_sequence<sizeof...(FArgs) + 1 - sizeof...(Args)>{}, f, std::forward<Args>(args)...);
}

template<class R, typename T, class... FArgs, class... Args>
auto easy_bind(R(T::*f)(FArgs...) const, Args&&... args)
    -> decltype(detail::easy_bind(std::make_integer_sequence<sizeof...(FArgs) + 1 - sizeof...(Args)>{}, f, std::forward<Args>(args)...))
{
    return detail::easy_bind(std::make_integer_sequence<sizeof...(FArgs) + 1 - sizeof...(Args)>{}, f, std::forward<Args>(args)...);
}
