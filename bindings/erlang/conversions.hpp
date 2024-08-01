#pragma once

namespace erl
{
    class invalid_argument : public std::invalid_argument { using std::invalid_argument::invalid_argument; };

    class atom : public std::string { using std::string::string; };

    template<typename T>
    ETERM* as_term(T&& value);

    template<typename T>
    T from_term(ETERM* term);

    namespace detail {
        template<typename... Ts, size_t... Is>
        ETERM* as_term_impl(std::tuple<Ts...>&& value, std::integer_sequence<Is...>) {
            ETERM* elements[] = { as_term(std::get<Is>(std::forward<std::tuple<Ts...>>(value)))... };
            ETERM* result = erl_mk_tuple(elements, sizeof...(Is));
            erl_free_array(elements, sizeof...(Is));
            return result;
        }
    }

    template<typename... Ts>
    ETERM* as_term(std::tuple<Ts...>&& value) {
        return detail::as_term_impl(std::forward<std::tuple<Ts...>>(value), std::make_integer_sequence<sizeof...(Ts)>());
    }

    ETERM* as_error(const std::string& message);
}
