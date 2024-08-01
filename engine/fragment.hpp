#pragma once

namespace ethelo
{
    class fragment : public expression
    {
        void compile();

    public:
        fragment();
        fragment(const std::string& name, const std::string& source);
        fragment(const fragment& other);
        fragment(fragment&& other);
        virtual ~fragment() {};
        fragment& operator=(const fragment& other);
    };
}
