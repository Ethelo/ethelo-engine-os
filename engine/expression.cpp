#include "ethelo.hpp"

namespace ethelo
{
    expression::expression()
        : type_(EXPRESSION)
    {};

    expression::expression(expression_type type)
        : type_(type)
    {};

    expression::expression(expression_type type, const std::string& name, const std::string& source)
        : name_(name), type_(type)
    { parse(source); }

    expression::expression(const std::string& name, const std::string& source)
        : name_(name), type_(EXPRESSION)
    { parse(source); }

    expression::expression(const expression& other)
        : name_(other.name_), type_(other.type_)
    { if (other.source_) parse(*other.source_); }

    expression::expression(expression&& other)
        : name_(std::move(other.name_)),
          type_(std::move(other.type_)),
          source_(std::move(other.source_)),
          input_(std::move(other.input_)),
          lexer_(std::move(other.lexer_)),
          tokens_(std::move(other.tokens_)),
          parser_(std::move(other.parser_))
    {}

    expression& expression::operator=(const expression& other)
    { name_ = other.name_; if (other.source_) parse(*other.source_); return *this; }

    void expression::parse(const std::string& source)
    {
        source_.reset(new std::string(source));
        input_.reset(antlr3StringStreamNew((pANTLR3_UINT8)source_->c_str(), ANTLR3_ENC_8BIT, source_->size(), (pANTLR3_UINT8)name_.c_str()));
        lexer_.reset(expressionLexerNew(input_.get()));
        tokens_.reset(antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer_.get())));
        parser_.reset(expressionParserNew(tokens_.get()));
        lexer_->pLexer->rec->displayRecognitionError = display_recognition_error;
        parser_->pParser->rec->displayRecognitionError = display_recognition_error;
        lexer_->pLexer->super = this;
        parser_->pParser->super = this;
        ast_ = parser_->start(parser_.get()).tree;
    }

    void expression::error(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8* tokenNames)
    {
        throw syntax_error(*this, recognizer->state->exception);
    }

    void expression::display_recognition_error(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8* tokenNames)
    {
        expression* instance = NULL;

        switch  (recognizer->type)
        {
        case ANTLR3_TYPE_LEXER:
            instance = (expression*)((pANTLR3_LEXER)recognizer->super)->super;
            break;

        case ANTLR3_TYPE_PARSER:
            instance = (expression*)((pANTLR3_PARSER)recognizer->super)->super;
            break;

        default:
            break;
        }

        if (instance)
            instance->error(recognizer, tokenNames);
    }

    std::string expression::to_string(pANTLR3_BASE_TREE node)
    {
        switch(node->getType(node))
        {
        case TOK_ID:
            return std::string((const char*)node->getText(node)->chars);
        case TOK_STRING: {
            std::string str((const char*)node->getText(node)->chars);
            return str.substr(1, str.size() - 2);
        }

        default:
            return std::string();
        }
    }

    uint64_t expression::to_integer(pANTLR3_BASE_TREE node)
    {
        switch(node->getType(node))
        {
        case TOK_INTEGER:
            return strtol((const char*)node->getText(node)->chars, NULL, 10);
        case TOK_FLOATING:
            return strtod((const char*)node->getText(node)->chars, NULL);

        default:
            return 0;
        }
    }

    double expression::to_float(pANTLR3_BASE_TREE node)
    {
        switch(node->getType(node))
        {
        case TOK_INTEGER:
            return strtol((const char*)node->getText(node)->chars, NULL, 10);
        case TOK_FLOATING:
            return strtod((const char*)node->getText(node)->chars, NULL);

        default:
            return 0;
        }
    }
}
