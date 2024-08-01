#pragma once

namespace ethelo
{
    class display : public expression
    {
        void compile();

    public:
        display();
        display(const std::string& name, const std::string& source);
        display(const display& other);
        display(display&& other);
        virtual ~display() {};
        display& operator=(const display& other);
    };
}
