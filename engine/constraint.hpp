#pragma once

namespace ethelo
{
    struct bound
    {
        double value = 0.0;
        bool enabled = false;

        void set(double value) { this->value = value; enabled = true; }

        operator bool() const { return enabled; }
        double get() const { return value; }
    };

    class constraint : public expression
    {
        bool relaxable_;
        bound lbound_, ubound_;

        void compile();
        void compile_bound(pANTLR3_BASE_TREE node);

    public:
        constraint();
        constraint(const std::string& name, const std::string& source, const bool &relaxable = true);
        constraint(const constraint& other);
        constraint(constraint&& other);
        virtual ~constraint() {};
        constraint& operator=(const constraint& other);

        const bound& lbound() const { return lbound_; }
        const bound& ubound() const { return ubound_; }
        bool is_relaxable() const { return relaxable_; }
    };
}
