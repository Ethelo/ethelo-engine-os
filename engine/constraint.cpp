#include "ethelo.hpp"

namespace ethelo
{
    constraint::constraint()
        : expression(CONSTRAINT)
    {};

    constraint::constraint(const std::string& name, const std::string& source, const bool &relaxable)
        : expression(CONSTRAINT, name, source), relaxable_(relaxable)
    { compile();};

    constraint::constraint(const constraint& other)
        : expression(other), relaxable_(other.relaxable_)
    { compile();}

    constraint::constraint(constraint&& other)
        : expression(std::move(other)), relaxable_(std::move(other.relaxable_)),lbound_(std::move(other.lbound_)), ubound_(std::move(other.ubound_))
    {}

    constraint& constraint::operator=(const constraint& other)
    { expression::operator=(other); compile(); return *this; }

    void constraint::compile()
    {
        if (!valid()) return;

        auto root = ast();
        if (root->getType(root) != TOK_INEQ)
            throw syntax_error(*this, root, "TypeError", "expected constraint expression");

        for (int i = 0; i < root->getChildCount(root); i++)
        {
            auto child = (pANTLR3_BASE_TREE)root->getChild(root, i);
            auto type = child->getType(child);
            if (type == TOK_LEQ || type == TOK_GEQ || type == TOK_EQ)
                compile_bound(child);
        }

        if (!lbound_ && !ubound_)
            throw semantic_error(*this, root, "NoBound", "expression is unbounded");
    }

    void constraint::compile_bound(pANTLR3_BASE_TREE node)
    {
        if (node->getChildCount(node) <= 0) return;
        double value = expression::to_float((pANTLR3_BASE_TREE)node->getChild(node, 0));

        switch (node->getType(node))
        {
        case TOK_GEQ:
            if (lbound_) throw semantic_error(*this, node, "DuplicateBound", "duplicate lower bound");
            lbound_.set(value);
            break;
        case TOK_LEQ:
            if (ubound_) throw semantic_error(*this, node, "DuplicateBound", "duplicate upper bound");
            ubound_.set(value);
            break;
        case TOK_EQ:
            if (lbound_ || ubound_) throw semantic_error(*this, node, "DuplicateBound", "equality mixed with inequality");
            lbound_.set(value);
            ubound_.set(value);
            break;

        default:
            break;
        }
    }
}
