#include "ethelo.hpp"

namespace ethelo
{
    fragment::fragment()
        : expression(FRAGMENT)
    {};

    fragment::fragment(const std::string& name, const std::string& source)
        : expression(FRAGMENT, name, source)
    { compile(); };

    fragment::fragment(const fragment& other)
        : expression(other)
    { compile(); }

    fragment::fragment(fragment&& other)
        : expression(std::move(other))
    {}

    fragment& fragment::operator=(const fragment& other)
    { expression::operator=(other); compile(); return *this; }

    void fragment::compile()
    {
        if (!valid()) return;

        auto root = ast();
        if (root->getType(root) != TOK_EXPR)
            throw syntax_error(*this, root, "TypeError", "expected fragment expression");
    }
}
