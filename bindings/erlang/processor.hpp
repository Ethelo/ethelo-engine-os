#pragma once

namespace ethelo
{
    template<typename F>
    class erlang_functor {
        using FF = typename function_traits<F>::free_function_type;
        std::function<FF> f;

        template<typename... Args>
        ETERM* apply(const std::function<void(Args...)>& f, typename std::decay<Args>::type&&... args) {
            f(std::forward<Args>(args)...);
            return erl_mk_atom("ok");
        }

        template<typename R, typename... Args>
        typename std::enable_if<!std::is_void<R>::value, ETERM*>::type
        apply(const std::function<R(Args...)>& f, typename std::decay<Args>::type&&... args) {
            return erl::as_term(f(std::forward<Args>(args)...));
        }

        template<size_t... Is>
        ETERM* apply(ETERM* arguments, std::integer_sequence<Is...>) {
            // Argument count
            constexpr size_t argc = sizeof...(Is);

            // Check number of arguments
            if (erl_size(arguments) != argc) {
                std::stringstream stream;
                stream << "Incorrect number of arguments (requires " << argc << " but provided "<< erl_size(arguments) << ")!";
                return erl::as_error(stream.str());
            }

            // Prepare arguments
            erl::unique_term argv[] = {erl::unique_term(erl_element(Is + 1, arguments))...};

            // Context info helper
            int last_argc = 0;
            ETERM* last_argv = NULL;
            auto get_arg = [&](int index) {
                last_argc = index;
                last_argv = argv[index].get();
                return last_argv;
            };

            // Execute function and return result
            ETERM* result = NULL;
            try {
                result = apply(f, erl::from_term<typename std::decay<typename function_traits<FF>::template argument<Is>::type>::type>(get_arg(Is))...);
            }
            catch(const erl::invalid_argument& e) {
                std::stringstream stream;
                stream << "Argument " << last_argc + 1 << " invalid: " << e.what();
                result = erl::as_error(stream.str());
            }
            catch(const std::exception& e) {
                result = erl::as_error(e.what());
            }
            return result;
        }

    public:
        erlang_functor(std::function<FF> func)
            : f(func) {};

        ETERM* operator() (ETERM* arguments) {
            return apply(arguments, std::make_integer_sequence<function_traits<FF>::arity>());
        }
    };

    class processor
    {
        bool _entered = false;
        bool _exit = false;
        std::unordered_map<std::string, std::function<ETERM* (ETERM*)>> _commands;

        static int write_term(ETERM* term);

    public:
        int main();

    protected:
        template<typename Klass>
        void add(std::string name, ETERM* (Klass::*function) (ETERM*)) {
            _commands[name] = std::bind(function, (Klass*) this, std::placeholders::_1);
        }

        template<typename Func>
        void bind(std::string name, Func func) {
            _commands[name] = std::bind(erlang_functor<Func>(easy_bind(func)), std::placeholders::_1);
        }

        template<typename Klass, typename Func>
        void bind(std::string name, Func func, Klass* instance) {
            _commands[name] = std::bind(erlang_functor<Func>(easy_bind(func, instance)), std::placeholders::_1);
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

        virtual ETERM* invoke(const std::string& name, const std::function<ETERM* (ETERM*)>& function, ETERM* arguments);

        virtual void on_init() {};
        virtual void on_exit() {};
    };
}
