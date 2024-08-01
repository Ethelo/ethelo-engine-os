#pragma once

namespace ethelo
{
    class decision : public problem
    {
        indexed_vector<criterion> criteria_;
        arma::mat influents_;
        arma::mat weights_;
        arma::mat local_weights_;
        arma::vec null_voted_options_;
        arma::uvec neutral_voted_options_;
        bool sat_range_;
        double sat_min_;
        double sat_max_;

        void load(const std::vector<option>& options,
                  const std::vector<criterion>& criteria,
                  const std::vector<fragment>& fragments,
                  const std::vector<constraint>& constraints,
                  const std::vector<display>& displays,
                  const arma::mat& influents,
                  const arma::mat& weights,
                  const arma::mat& exclusions,
                  const configuration& config);

        void range();

        arma::uvec total_votes(arma::vec x) const;
        arma::vec expand(arma::vec x) const;
        arma::mat weighted_influents(arma::vec x, bool global) const;
        arma::vec satisfaction(const arma::mat& influents, arma::vec x, bool global=false) const;
        arma::vec satisfaction(arma::vec x, bool global=false) const;

        arma::uvec null_vote_option_indices() const;
        arma::vec vote_counts(const arma::mat& influents) const;
        arma::mat transform_nullvotes(const arma::mat& influents);
        arma::mat transform_weights(const arma::mat& influents, const arma::mat& weights) const;
        arma::uvec transform_neutralvotes(const arma::mat& influents);

    public:
        decision();
        decision(const std::vector<option>& options,
                 const std::vector<criterion>& criteria,
                 const std::vector<fragment>& fragments,
                 const std::vector<constraint>& constraints,
                 const std::vector<display>& displays,
                 const arma::mat& influents = arma::mat(),
                 const arma::mat& weights = arma::mat(),
                 const arma::mat& exclusions = arma::mat(),
                 const configuration& config = {});
        decision(const decision& other);
        decision(decision&& other);
        virtual ~decision() {};
        decision& operator=(const decision& other);

        void load(const arma::mat& influents, const arma::mat& weights);
        void configure(const configuration& config);

        stats statistics(arma::vec x, bool global=false) const;
        solution solve();

        const indexed_vector<criterion>& criteria() const { return criteria_; }
        const arma::mat& influents() const { return influents_; }
        const arma::mat& weights() const { return weights_; }
    };

    constexpr double null_vote = 2.0f;
    constexpr double NULL_VOTE_NOMINAL_PENALTY = -1E-5;
    constexpr double NEUTRAL_VOTE_NOMINAL_PENALTY = -1E-5;
    constexpr double NULL_VOTE_SUPPORT_ONLY_REPLACEMENT_VALUE = 0.5;
    constexpr double NULL_VOTE_REPLACEMENT_VALUE = 0.0;
}
