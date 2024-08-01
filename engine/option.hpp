#pragma once

namespace ethelo
{
    class option
    {
        std::string name_;
        indexed_vector<detail> details_;
        bool determinative_;
    public:
        option() {};
        option(const std::string& name, const std::vector<detail>& details = {}, const bool &determinative = false)
            : name_(name), determinative_(determinative) { details_.load(details); }
        virtual ~option(){};

        double get_detail(const std::string& name) const {
          auto idx = details().find(name);
          return idx >= 0 ? details()[idx].value() : 0.0;
        }

        std::string name() const { return name_; }
        const indexed_vector<detail>& details() const { return details_; }
        bool determine() const { return determinative_; }
    };
}
