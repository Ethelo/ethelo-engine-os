#pragma once

#include "language/expressionLexer.h"
#include "language/expressionParser.h"

namespace ethelo
{
    class expression
    {
    public:
        enum expression_type { EXPRESSION, FRAGMENT, CONSTRAINT, DISPLAY };

    private:
        static void display_recognition_error(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8* tokenNames);

        struct deleter {
            template<typename T>
            void operator()(T* obj) { obj->free(obj); }
        };

        std::string name_;
        expression_type type_;
        std::unique_ptr<std::string> source_;
        std::unique_ptr<ANTLR3_INPUT_STREAM, deleter> input_;
        std::unique_ptr<ANTLR3_COMMON_TOKEN_STREAM, deleter> tokens_;
        std::unique_ptr<expressionLexer, deleter> lexer_;
        std::unique_ptr<expressionParser, deleter> parser_;
        pANTLR3_BASE_TREE ast_;

        void parse(const std::string& source);
        void error(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8* tokenNames);

    public:
        expression();
        expression(expression_type type);
        expression(expression_type type, const std::string& name, const std::string& source);
        expression(const std::string& name, const std::string& source);
        expression(const expression& other);
        expression(expression&& other);
        virtual ~expression() {};
        expression& operator=(const expression& other);

        operator bool() const { return valid(); }
        bool valid() const { return ast() != NULL; }
        std::string name() const { return name_; }
        expression_type type() const { return type_; }
        std::string source() const { return source_ ? *source_ : ""; }
        pANTLR3_BASE_TREE ast() const { return parser_ ? ast_ : NULL; }

        static std::string to_string(pANTLR3_BASE_TREE node);
        static uint64_t to_integer(pANTLR3_BASE_TREE node);
        static double to_float(pANTLR3_BASE_TREE node);
    };
}
