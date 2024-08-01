#pragma once

namespace ethelo
{
    class criterion
    {
        std::string name_;

    public:
        criterion() {};
        criterion(const std::string& name) : name_(name) {};
        virtual ~criterion() {};

        std::string name() const { return name_; }
    };
}
