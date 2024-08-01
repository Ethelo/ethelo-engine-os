#pragma once

namespace ethelo
{
    class compile_error : public std::exception
    {
        const expression* expr_;
        std::string source_;
        uint32_t line_;
        int32_t position_;
        std::string type_;
        std::string message_;
        std::string what_;

    public:
        compile_error(const expression& expr, uint32_t line, int32_t position, const std::string& type, const std::string& message)
            : expr_(&expr), source_(expr.source()), line_(line), position_(position), type_(type), message_(message)
        {
            std::stringstream ss;
            if (expr_->type() == expression::FRAGMENT) ss << "@";
            ss << expr_->name() << "[" << line << ":" << position << "] : " << type;
            if (!message.empty()) ss << ": " << message;
            what_ = ss.str();
        }

        virtual ~compile_error() {}
        virtual const char* what() const noexcept { return what_.c_str(); }

        const expression& expr() const { return *expr_; }
        uint32_t line() const { return line_; }
        int32_t position() const { return position_; }
        const std::string& type() const { return type_; }
        const std::string& message() const { return message_; }
        const std::string& source() const { return source_; }
    };

    class syntax_error : public compile_error
    {
    public:
        syntax_error(const expression& expr, pANTLR3_EXCEPTION ex)
            : compile_error(expr, ex->line, ex->charPositionInLine, (const char*) ex->name, (const char*) ex->message) {}
        syntax_error(const expression& expr, pANTLR3_BASE_TREE node, const std::string& type, const std::string& message)
            : compile_error(expr, node->getLine(node), node->getCharPositionInLine(node), type, message) {}
    };

    class semantic_error : public compile_error
    {
    public:
        semantic_error(const expression& expr, pANTLR3_BASE_TREE node, const std::string& type, const std::string& message)
            : compile_error(expr, node->getLine(node), node->getCharPositionInLine(node), type, message) {}
    };

    class unknown_function : public std::invalid_argument
    {
    public:
        using std::invalid_argument::invalid_argument;
    };

    class invalid_access_to_blacklisted_detail : public std::out_of_range
    {
    public:
        using std::out_of_range::out_of_range;
    };
}
