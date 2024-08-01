#pragma once

namespace ethelo
{
    class detail
    {
        std::string name_;
        double value_;

    public:
        detail() {}
        detail(const std::string& name, double value) : name_(name), value_(value) {}
        virtual ~detail() {};

        std::string name() const { return name_; }
        double value() const { return value_; }
    };
}
