#pragma once
#include <ei.h>
#include <unordered_map>
#include <functional>
#include <sstream>
#include "conversions.hpp"
#include "util.hpp"

namespace ethelo
{
    // Custom index_sequence implementation
    template<std::size_t... Is>
    struct index_sequence {};

    template<std::size_t N, std::size_t... Is>
    struct make_index_sequence_impl : make_index_sequence_impl<N-1, N-1, Is...> {};

    template<std::size_t... Is>
    struct make_index_sequence_impl<0, Is...> {
        using type = index_sequence<Is...>;
    };

    template<std::size_t N>
    using make_index_sequence = typename make_index_sequence_impl<N>::type;

    template<typename F>
    class erlang_functor {
        using FF = typename function_traits<F>::free_function_type;
        std::function<FF> f;

        template<typename R, typename... Args>
        typename std::enable_if<std::is_void<R>::value, void>::type
        apply_impl(ei_x_buff* buff, const std::function<R(Args...)>& f, Args&&... args) {
            f(std::forward<Args>(args)...);
            ei_x_encode_atom(buff, "ok");
        }

        template<typename R, typename... Args>
        typename std::enable_if<!std::is_void<R>::value, void>::type
        apply_impl(ei_x_buff* buff, const std::function<R(Args...)>& f, Args&&... args) {
            erl::encode_term(buff, f(std::forward<Args>(args)...));
        }

        template<std::size_t... Is>
        void apply(const char* buf, int* index, ei_x_buff* result, index_sequence<Is...>) {
            constexpr size_t argc = sizeof...(Is);

            int arity;
            if (ei_decode_tuple_header(buf, index, &arity) < 0 || static_cast<size_t>(arity) != argc) {
                std::stringstream stream;
                stream << "Incorrect number of arguments (requires " << argc << " but provided " << arity << ")!";
                erl::encode_error(result, stream.str());
                return;
            }

            try {
                apply_impl(result, f, erl::decode_term<typename std::decay<typename function_traits<FF>::template argument<Is>::type>::type>(buf, index)...);
            } catch(const erl::invalid_argument& e) {
                std::stringstream stream;
                stream << "Argument " << (sizeof...(Is) - argc + 1) << " invalid: " << e.what();
                erl::encode_error(result, stream.str());
            } catch(const std::exception& e) {
                erl::encode_error(result, e.what());
            }
        }

    public:
        erlang_functor(std::function<FF> func) : f(func) {}

        void operator() (const char* buf, int* index, ei_x_buff* result) {
            apply(buf, index, result, make_index_sequence<function_traits<FF>::arity>{});
        }
    };
    class processor
    {
        bool _entered = false;
        bool _exit = false;
        std::unordered_map<std::string, std::function<void (const char*, int*, ei_x_buff*)>> _commands;

        static int write_term(ei_x_buff* buff);

    public:
        int main();

    protected:
        template<typename Klass>
        void add(std::string name, void (Klass::*function) (const char*, int*, ei_x_buff*)) {
            _commands[name] = std::bind(function, (Klass*) this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        template<typename Func>
        void bind(std::string name, Func func) {
            _commands[name] = std::bind(erlang_functor<Func>(easy_bind(func)), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        template<typename Klass, typename Func>
        void bind(std::string name, Func func, Klass* instance) {
            _commands[name] = std::bind(erlang_functor<Func>(easy_bind(func, instance)), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        }

        void remove(std::string name) {
            auto icmd = _commands.find(name);
            if (icmd != _commands.end())
                _commands.erase(icmd);
        }

        void clear() {
            _commands.clear();
        }

        void terminate() {
            _exit = true;
        }

        virtual void invoke(const std::string& name, const std::function<void (const char*, int*, ei_x_buff*)>& function, const char* buf, int* index, ei_x_buff* result);

        virtual void on_init() {}
        virtual void on_exit() {}
    };
}
