#include "ethelo.hpp"

namespace ethelo
{
    display::display()
        : expression(DISPLAY)
    {};

    display::display(const std::string& name, const std::string& source)
        : expression(DISPLAY, name, source)
    { compile(); };

    display::display(const display& other)
        : expression(other)
    { compile(); }

    display::display(display&& other)
        : expression(std::move(other))
    {}

    display& display::operator=(const display& other)
    { expression::operator=(other); compile(); return *this; }

    void display::compile()
    {
        if (!valid()) return;

        auto root = ast();
        if (root->getType(root) != TOK_EXPR)
            throw syntax_error(*this, root, "TypeError", "expected display expression");        
    }
}
