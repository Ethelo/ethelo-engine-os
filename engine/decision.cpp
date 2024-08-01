#include "ethelo.hpp"

namespace ethelo
{
    decision::decision()
    {}

    decision::decision(const std::vector<option>& options,
                       const std::vector<criterion>& criteria,
                       const std::vector<fragment>& fragments,
                       const std::vector<constraint>& constraints,
                       const std::vector<display>& displays,
                       const arma::mat& influents,
                       const arma::mat& weights,
                       const arma::mat& exclusions,
                       const configuration& config)
        : sat_range_(false), sat_min_(0), sat_max_(0)
    { load(options, criteria, fragments, constraints, displays, influents, weights, exclusions, config); }

    decision::decision(const decision& other)
        : problem(other),
          criteria_(other.criteria_),
          influents_(other.influents_),
          weights_(other.weights_),
          local_weights_(other.local_weights_),
          sat_range_(other.sat_range_),
          sat_min_(other.sat_min_),
          sat_max_(other.sat_max_)
    {}

    decision::decision(decision&& other)
        : problem(std::move(other)),
          criteria_(std::move(other.criteria_)),
          influents_(std::move(other.influents_)),
          weights_(std::move(other.weights_)),
          local_weights_(std::move(other.local_weights_)),
          sat_range_(other.sat_range_),
          sat_min_(other.sat_min_),
          sat_max_(other.sat_max_)
    {}

    decision& decision::operator=(const decision& other)
    {
        this->problem::operator=(other);
        criteria_ = other.criteria_;
        influents_ = other.influents_;
        weights_ = other.weights_;
        local_weights_ = other.local_weights_;
        sat_range_ = other.sat_range_;
        sat_min_ = other.sat_min_;
        sat_max_ = other.sat_max_;
        return *this;
    }

    void decision::load(const std::vector<option>& options,
                        const std::vector<criterion>& criteria,
                        const std::vector<fragment>& fragments,
                        const std::vector<constraint>& constraints,
                        const std::vector<display>& displays,
                        const arma::mat& influents,
                        const arma::mat& weights,
                        const arma::mat& exclusions,
                        const configuration& config)
    {
        if (options.size() == 0)
            throw std::invalid_argument("a decision must have at least one option");

        if (criteria.size() != 0)
            criteria_.load(criteria);
        else
            criteria_.load({criterion("default")});
        
        // Load & configure problem and load influents & weights
        problem::operator=(problem(options, fragments, constraints, displays));
        configure(config);
        load(influents, weights);
        exclude(exclusions);
    }

    void decision::range() {
        // Skip range discovery if already completed, or disabled
        if (sat_range_ || !config().discover_range) return;

        // Setup min/max influents
        arma::mat min_influent;
        arma::mat max_influent(arma::mat(1, decision::options().size() * decision::criteria().size(), arma::fill::ones));
        if (config().support_only)
            min_influent = arma::mat(arma::mat(1, decision::options().size() * decision::criteria().size(), arma::fill::zeros));
        else
            min_influent = arma::mat(arma::mat(1, decision::options().size() * decision::criteria().size(), arma::fill::ones) * -1);

        // Setup decision
        decision dec(*this);
        configuration conf;

        // Do not exclude any solutions
        dec.exclude(arma::mat());

        // Solve min_influent decision
        PLOGD << "Solve min influent";
        dec.load(min_influent, arma::mat());
        auto weighted_min = dec.weighted_influents(arma::vec(), true);
        conf.minimize = true; dec.configure(conf);
        auto solution_min_ = dec.solve();

        // Solve max_influent decision
        PLOGD << "Solve max influent";
        dec.load(max_influent, arma::mat());
        auto weighted_max = dec.weighted_influents(arma::vec(), true);
        conf.minimize = false; dec.configure(conf);
        auto solution_max_ = dec.solve();

        // Prepare results
        if (solution_min_.success && solution_max_.success) {
            sat_min_ = arma::mean(dec.satisfaction(weighted_min, solution_min_.x, true));
            sat_max_ = arma::mean(dec.satisfaction(weighted_max, solution_max_.x, true));
            sat_range_ = true;
        } else {
            throw std::runtime_error("failed to discover satisfaction range");
        }
        PLOGD << "Solved max influent";
    }

    void decision::load(const arma::mat& influents, const arma::mat& weights)
    {
        size_t num_options = options().size();
        size_t num_criteria = criteria().size();

        if (weights.size() == 0) {
            auto mat = arma::mat(influents.n_rows, num_options * num_criteria);
            mat.fill(1.0/(num_options * num_criteria));
            weights_ = arma::mat(mat);
        }
        else {
            if (weights.n_cols != num_options * num_criteria)
                throw std::invalid_argument("number of weight matrix columns does not match [number of options] * [number of criteria]");
            if (weights.n_rows != influents.n_rows)
                throw std::invalid_argument("number of weight matrix rows does not match number of influent matrix rows");
            weights_ = arma::mat(arma::normalise(arma::clamp(arma::mat(weights), 0.0, 1.0), 1, 1));
            local_weights_ = arma::mat(arma::normalise(arma::clamp(arma::mat(transform_weights(influents, weights)), 0.0, 1.0), 1, 1));
        }

        if (influents.size() == 0) {
            influents_ = arma::mat(0, num_options * num_criteria);
            arma::mat problem_influents(0, num_options);
            problem::load(problem_influents);
        }
        else {
            if (influents.n_cols != num_options * num_criteria)
                throw std::invalid_argument("number of influent matrix columns does not match [number of options] * [number of criteria]");

            // Transform the null votes to zero (among other things)
            influents_ = arma::clamp(transform_nullvotes(influents), -1.0, 1.0);

            arma::mat problem_influents(influents_.n_rows, num_options);
            for (size_t i = 0; i < num_options; i++)
                problem_influents.col(i)
                    = arma::sum(weights_.cols(i * num_criteria, (i + 1) * num_criteria - 1) %
                                influents_.cols(i * num_criteria, (i + 1) * num_criteria - 1), 1);

            if (config().normalize_influents)
                problem::load(arma::mat(arma::normalise(arma::mat(problem_influents), 1, 1)));
            else
                problem::load(problem_influents);
        }
    }

    void decision::configure(const configuration& config) {
        problem::configure(config);
        if (config.discover_range) range();
        else sat_range_ = false;
    }

    arma::vec decision::expand(arma::vec x) const
    {
        size_t num_options = options().size();
        size_t num_criteria = criteria().size();

        if (x.size() == num_options) {
            arma::vec expanded(num_options * num_criteria);
            for (size_t i = 0; i < num_options; i++)
                for (size_t j = 0; j < num_criteria; j++)
                    expanded(i * num_criteria + j) = x(i);
            return expanded;
        }
        else if (x.size() != num_options * num_criteria) {
            throw std::invalid_argument("invalid independent variable size");
        }
        else {
            return x;
        }
    }

    arma::mat decision::weighted_influents(arma::vec x, bool global) const
    {
        if (influents().size() == 0)
            throw std::runtime_error("no influent data");

        arma::mat normalized_weights;

        if (global)
            normalized_weights = arma::mat(weights_);
        else {
            normalized_weights = arma::mat(local_weights_);
            normalized_weights %= arma::repmat(x.t(), local_weights_.n_rows, 1);
        }

        normalized_weights = arma::normalise(normalized_weights, 1, 1);
        return normalized_weights % arma::mat(influents());
    }

    arma::uvec decision::total_votes(arma::vec x) const
    {
        if (influents().size() == 0)
            throw std::runtime_error("no influent data");

        arma::mat localized_weights = arma::mat(local_weights_);
        localized_weights %= arma::repmat(expand(x).t(), local_weights_.n_rows, 1);
        return arma::find(arma::clamp(arma::ceil(arma::vec(arma::max(arma::abs(arma::mat(localized_weights)), 1))), 0.0, 1.0));
    }

    arma::vec decision::satisfaction(const arma::mat& influents, arma::vec x, bool global) const
    {
        arma::vec sat = arma::clamp(influents * expand(x), -1, 1);

        if (global && config().per_option_satisfaction) {
            double total = arma::sum(x);
            if (total > std::numeric_limits<double>::epsilon())
                return sat / total;
            else
                return sat * 0.0;
        } else {
            return sat;
        }
    }

    arma::vec decision::satisfaction(arma::vec x, bool global) const
    {
        return satisfaction(weighted_influents(expand(x), global), x, global);
    }

    arma::uvec decision::null_vote_option_indices() const {
      return arma::find(null_voted_options_);
    }

    arma::vec decision::vote_counts(const arma::mat& influents) const
    {
      arma::mat votes_status(influents);
      votes_status.transform([](double val) {
          return std::abs(val - null_vote) <= std::numeric_limits<double>::epsilon()
              ? 0.0 : 1.0;
      });

      return arma::vec(arma::sum(votes_status).t());
    }

    arma::mat decision::transform_nullvotes(const arma::mat& influents)
    {
        // Get the (linear) indices of the null entries
        arma::uvec inds = arma::find(arma::abs(influents - null_vote) <= std::numeric_limits<double>::epsilon());

        double null_vote_replacement_value = NULL_VOTE_REPLACEMENT_VALUE;
        if(config().support_only)
          null_vote_replacement_value = NULL_VOTE_SUPPORT_ONLY_REPLACEMENT_VALUE;

        // Copy the influents matrix and replace the null votes with
        arma::mat votes_transformed(influents);
        votes_transformed(inds).fill(null_vote_replacement_value);

        arma::vec votes_counts = vote_counts(influents);

        size_t num_criteria = criteria_.size();
        arma::mat vote_counts_per_option = arma::sum(arma::reshape(votes_counts, num_criteria, votes_counts.size() / num_criteria));
        null_voted_options_ = arma::vec(vote_counts_per_option.transform([](double val) {
            return val > 0.0 ? 0.0 : 1.0; 
        }).t());

        arma::vec votes_averages = arma::vec(arma::sum(votes_transformed).t());
        for (size_t i = 0; i < votes_averages.size(); i++) {
            if(votes_counts(i) > 0.0) {
                votes_averages(i) = votes_averages(i) /  votes_counts(i);
            } else {
                // if an option hasn't been voted on at all, adjust to prefer it less than a neutral vote 
                votes_averages(i) = null_vote_replacement_value + NULL_VOTE_NOMINAL_PENALTY;
            }
        }

        // Find all neutral voted options (i.e. the original influent
        // values equal to zero when the range is [-1,1] or 1/2 when
        // the range is [0,1])
        arma::umat pure_neutral_votes = arma::abs(influents-null_vote_replacement_value) <= std::numeric_limits<double>::epsilon();

        // Find the mixed columns containing only null or neutral votes
        neutral_voted_options_ = arma::all(arma::abs(votes_transformed-null_vote_replacement_value) <= std::numeric_limits<double>::epsilon()).t();

        // Set the pure neutral vote columns with valid (non-null or neutral) entries to zero
        pure_neutral_votes.cols(arma::find(neutral_voted_options_ == 0)).zeros();

        // Find the remaining neutral vote (linear) indices. These are
        // the entries that should be peanalized
        arma::uvec mixed_neutral_null_votes = arma::find(pure_neutral_votes);

        // Set the neutral voted entries to some small negative number
        votes_transformed(mixed_neutral_null_votes).fill(null_vote_replacement_value + NEUTRAL_VOTE_NOMINAL_PENALTY);

        // Replace each null vote with its column average.
        votes_transformed(inds) = votes_averages(inds/influents.n_rows);
        return votes_transformed;
    }

    arma::mat decision::transform_weights(const arma::mat& influents, const arma::mat& weights) const
    {
        arma::mat weights_transformed(weights);
        weights_transformed(arma::find( arma::abs(influents - null_vote) <= std::numeric_limits<double>::epsilon())).zeros();
        return weights_transformed;
    }

    inline double ethelo_function(double support, double dissonance, double collective_identity, double tipping_point) {
        if (std::abs(support) <= std::numeric_limits<double>::epsilon())
            return 0.0;

        double K;
        if(dissonance > tipping_point)
            K = support / (1.0 - tipping_point);
        else
            K = (support > 0.0) ? ((1.0 - support) / tipping_point) : ((-1.0 - support) / tipping_point);

        double ethelo = support + collective_identity * (tipping_point - dissonance) * K;
        ethelo = std::max(ethelo, -1.0);
        ethelo = std::min(ethelo, 1.0);
        return ethelo;
    }

    stats decision::statistics(arma::vec x, bool global) const
    {
        stats statistics;

        // Dimensions sanity check
        if (global && x.size() != options().size())
            throw std::invalid_argument("expected independent variable size to be number of options");
        if (x.size() != options().size() &&
            x.size() != options().size() * criteria().size())
            throw std::invalid_argument("invalid independent variable size");

        // Expand input as needed
        statistics.x = expand(x);

        // Compute satisfaction
        arma::vec sat = satisfaction(x, global);

        // Compute total votes
        arma::uvec valid_votes=total_votes(x);
        double total = global ? sat.size() : valid_votes.size();

        // Bail out if there are no votes
        if (std::abs(total) <= std::numeric_limits<double>::epsilon()) {
            statistics.data["ethelo"] = 0.0;
            statistics.data["support"] = 0.0;
            statistics.data["approval"] = 0.0;
            statistics.data["dissonance"] = 0.0;
            statistics.data["fairness"] = 0.0;
            statistics.data["total_votes"] = 0.0;
            statistics.data["abstain_votes"] = std::max(sat.size() - total, 0.0);
            statistics.data["negative_votes"] = 0.0;
            statistics.data["neutral_votes"] = 0.0;
            statistics.data["positive_votes"] = 0.0;
            statistics.histogram = arma::uvec(config().histogram_bins, arma::fill::zeros);
            return statistics;
        }

        // Scale satisfaction according to computed range
        if (global && sat_range_) {
            for(size_t i = 0; i < sat.size(); i++) {
                if (sat(i) < 0)
                    sat(i) = sat(i) / std::abs(sat_min_);
                else
                    sat(i) = sat(i) / std::abs(sat_max_);
            }
        }

        // Perform a clamp once more
        sat = arma::clamp(sat, -1, 1);

        // Compute the support statistic
        statistics.data["support"] = arma::sum(sat) / total;

        // Compute the approval & vote statistics
        double positive = 0.0, neutral = 0.0, negative = 0.0;
        for(double score : sat) {
            if (std::abs(score) <= std::abs(NULL_VOTE_NOMINAL_PENALTY) + std::numeric_limits<double>::epsilon())
                neutral += 1.0;
            else if (score > 0)
                positive += 1.0;
            else
                negative += 1.0;
        }
        statistics.data["approval"] = positive/total;
        statistics.data["total_votes"] = total;
        statistics.data["abstain_votes"] = std::max(sat.size() - total, 0.0);
        statistics.data["negative_votes"] = negative;
        statistics.data["neutral_votes"] = std::max(neutral - (sat.size() - total), 0.0);
        statistics.data["positive_votes"] = positive;

        // Compute the dissonance statistic
        statistics.data["dissonance"] = 0.0;
        if (total > 1)
            statistics.data["dissonance"] = arma::var(sat(valid_votes));

        // Compute the Ethelo score
        statistics.data["ethelo"] = ethelo_function(statistics.data["support"], statistics.data["dissonance"],
                                                    config().collective_identity, config().tipping_point);

        // If a collective identity is set, compute fairness
        if (config().collective_identity >= std::numeric_limits<double>::min() && total > 1)
            statistics.data["fairness"] = statistics.data["ethelo"] - statistics.data["support"];

        // Compute the histogram
        statistics.histogram = arma::uvec(config().histogram_bins, arma::fill::zeros);
        for (double score : sat) {
            size_t bin = std::min((size_t) std::floor((score + 1.0)/2.0 * statistics.histogram.size()),
                                  (size_t) statistics.histogram.size() - 1);
            statistics.histogram(bin) += 1.0;
        }
        statistics.histogram(std::floor(statistics.histogram.size()/2.0)) = statistics.data["neutral_votes"];

        return statistics;
    }

    solution decision::solve() {
        PLOGD << "Starting solve";
        // first try with null-voted options eliminated, both to make sure they're excluded and for perfomance
        const arma::uvec& null_vote_exclusions = null_vote_option_indices(); 
        exclude(null_vote_exclusions);

        PLOGD << "Starting bonmin solver";
        solution s = solver().solve(*this);
        PLOGD << "Solution status: " << s.status;

        // clear the exclusions
        PLOGD << "Clearing option exclusions";
        exclude(arma::uvec({}));

        // if solution not found, maybe we needed those options....try again without them
        if(s.status.compare("infeasible") == 0 || s.status.compare("unknown_solver_error") == 0) {
            PLOGD << "Trying solver again without option exclusions";
            s = solver().solve(*this);
          PLOGD << "Solution status: " << s.status;
        } 
        PLOGD << "Solve complete";
        return s;
    }
}
