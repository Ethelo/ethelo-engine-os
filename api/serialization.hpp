#pragma once

namespace ethelo
{
    class serializer_base
    {
    protected:
        static bool init_;
        static void init();
    };

    template<typename Ty>
    class serializer : public serializer_base
    {
    public:
        class not_found : public std::runtime_error { using std::runtime_error::runtime_error; };
        class parse_error : public std::runtime_error { using std::runtime_error::runtime_error; };

        virtual Ty deserialize(const std::string& text) = 0;
        virtual std::string serialize(const Ty& obj) = 0;
        virtual ~serializer() {};

        static std::unique_ptr<serializer<Ty>> create(const std::string& name) {
            if (!init_) { init(); init_ = true; }
            auto it = constructors_.find(name);
            if (it == constructors_.end())
                throw not_found("'" + name + "' is not a valid serializer");
            return it->second();
        }

    protected:
        typedef std::unique_ptr<serializer<Ty>> create_function();
        static void bind(const std::string& name, create_function* func) {
            constructors_[name] = func;
        }

    private:
        static std::map<std::string, std::function<create_function>> constructors_;
    };

    template<typename Ty>
    std::map<std::string, std::function<typename serializer<Ty>::create_function>> serializer<Ty>::constructors_;

    template<typename Ty, typename Serializer>
    class serializer_impl : public serializer<Ty>
    {
    public:
        static std::unique_ptr<serializer<Ty>> create() {
            return std::unique_ptr<serializer<Ty>>(new Serializer());
        }

        static void bind() {
            serializer<Ty>::bind(Serializer::name(), &create);
        }
    };
}
