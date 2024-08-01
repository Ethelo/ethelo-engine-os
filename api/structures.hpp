#pragma once

namespace ethelo
{
    struct solver_config {
        solver_config(bool single_outcome = false,
                      bool support_only = false,
                      bool normalize_satisfaction = true,
                      bool normalize_influents = false,
                      double collective_identity = 0.0,
                      double tipping_point = 1.0/3.0,
                      size_t histogram_bins = 5,
                      size_t solution_limit = 10)
            : single_outcome(single_outcome),
              support_only(support_only),
              normalize_satisfaction(normalize_satisfaction),
              normalize_influents(normalize_influents),
              collective_identity(collective_identity),
              tipping_point(tipping_point),
              histogram_bins(histogram_bins),
              solution_limit(solution_limit)
        {};

        bool single_outcome;
        bool support_only;
        bool per_option_satisfaction;
        bool normalize_satisfaction;
        bool normalize_influents;
        double collective_identity;
        double tipping_point;
        size_t histogram_bins;
        size_t solution_limit;
        std::set<std::string> issues;
    };

    class result {
        decision* decision_;
        configuration config_;
        solution solution_;
        size_t exclusions_;
        bool global_;

    public:
        result(decision& decision,
               solution& solution,
               size_t exclusions,
               bool global)
            : decision_(&decision),
              config_(decision.config()),
              solution_(solution),
              exclusions_(exclusions),
              global_(global)
        {}

        void activate_config() const { decision_->configure(config_); }
        decision& get_decision() const { return *decision_; }
        const configuration& get_config() const { return config_; }
        const solution& get_solution() const { return solution_; }
        size_t get_exclusions() const { return exclusions_; }
        size_t get_global() const { return global_; }
    };

    struct result_set {
        solver_config config;
        std::vector<result> results;
    };
}
