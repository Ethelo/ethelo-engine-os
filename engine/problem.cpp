#include "ethelo.hpp"
#include "MathModel/MathProgram.hpp" // for assertions in linkMathProgram
namespace ethelo
{
    problem::problem()
    {}

    problem::problem(const std::vector<option>& options,
                     const std::vector<fragment>& fragments,
                     const std::vector<constraint>& constraints,
                     const std::vector<display>& displays,
                     const arma::mat& influents,
                     const arma::mat& exclusions,
                     const configuration& config)
    { configure(config); load(options, fragments, constraints, displays, influents, exclusions); }

    problem::problem(const problem& other)
        : options_(other.options_),
          fragments_(other.fragments_),
          constraints_(other.constraints_),
          displays_(other.displays_),
          influents_(other.influents_),
          exclusions_(other.exclusions_),
          config_(other.config_),
		  preproc_MP(other.preproc_MP)
    {}

    problem::problem(problem&& other)
        : options_(std::move(other.options_)),
          fragments_(std::move(other.fragments_)),
          constraints_(std::move(other.constraints_)),
          displays_(std::move(other.displays_)),
          influents_(std::move(other.influents_)),
          exclusions_(std::move(other.exclusions_)),
          config_(std::move(other.config_)),
		  preproc_MP(std::move(other.preproc_MP))
    {}

    problem& problem::operator=(const problem& other)
    {
        options_ = other.options_;
        fragments_ = other.fragments_;
        constraints_ = other.constraints_;
        displays_ = other.displays_;
        influents_ = other.influents_;
        exclusions_ = other.exclusions_;
        config_ = other.config_;
		preproc_MP = other.preproc_MP;
        return *this;
    }

    void problem::load(const std::vector<option>& options,
                       const std::vector<fragment>& fragments,
                       const std::vector<constraint>& constraints,
                       const std::vector<display>& displays,
                       const arma::mat& influents,
                       const arma::mat& exclusions)
    {
        if (options.size() == 0)
            throw std::invalid_argument("a problem must have at least one option");

        options_.load(options);
        fragments_.load(fragments);
        constraints_.load(constraints);
        displays_.load(displays);
        load(influents);
        exclude(exclusions);
    }

    void problem::load(const arma::mat& influents)
    {
        if (influents.size() == 0)
            influents_ = arma::mat(0, options().size());
        else {
            if (influents.n_cols != options().size())
                throw std::invalid_argument("number of influent matrix columns does not match number of options");
            influents_ = influents;
        }
    }


    void problem::load(const std::vector<constraint>& constraints)
    {
        constraints_.load(constraints);
    }

    void problem::exclude(const arma::mat& exclusions)
    {
        if (exclusions.size() == 0)
          exclusions_ = arma::mat(0, options().size());
        else {
            if (exclusions.n_cols != options().size())
                throw std::invalid_argument("number of exclusion matrix columns does not match number of options");
            exclusions_ = exclusions;
        }
    }

    void problem::exclude(const arma::uvec& exclusions) {
        excluded_details_.clear();

        if(exclusions.empty()) {
            options_in_scope_.clear();
            return;
        }

        size_t whitelist_size = options_.size()-exclusions.size();
        size_t criteria_size = influents_.n_cols / options_.size(); 
        std::vector<option> scoped_options(whitelist_size);
        exclusions_in_scope_ = arma::mat(exclusions_.n_rows, whitelist_size); //scenario exclusions
        influents_in_scope_ = arma::mat(influents_.n_rows, whitelist_size*criteria_size);
        original_option_indexes_.resize(whitelist_size);

        std::map<std::string, double> included_option_detail_sums;
        std::map<std::string, double> excluded_option_detail_sums;

        size_t target_i = 0;
        for(size_t i = 0; i < options_.size(); i++) {
            const auto& cur_option = options_[i];
            std::map<std::string, double>* detail_sums_ptr;

            if(!arma::any(exclusions == i)) {
                PLOGD << "Including option: " << cur_option.name();
                exclusions_in_scope_.col(target_i) = exclusions_.col(i);
                for(size_t j = 0; j < criteria_size; j++)
                  influents_in_scope_.col(j*criteria_size+target_i) = influents_.col(j*criteria_size+i);
                scoped_options[target_i] = options_[i];
                original_option_indexes_[target_i] = i;

                detail_sums_ptr = &included_option_detail_sums; 
                target_i++;
            } else {
                PLOGD << "Excluding option: " << cur_option.name();
                detail_sums_ptr = &excluded_option_detail_sums; 
            }

            for(auto cur_detail : cur_option.details()) {
                (*detail_sums_ptr)[cur_detail.name()] += std::abs(cur_detail.value());
            }
        }

        options_in_scope_.load(scoped_options);

        // exclude details where:
        // 1) they are not set, or are 0, for in-scope options; and
        // 2) there is an excluded option that is non-zero; constraints with these excluded details will be relaxed 
        for (std::pair<std::string, double> excluded_sum : excluded_option_detail_sums) {
            if(std::abs(excluded_sum.second) > std::numeric_limits<double>::epsilon() &&
               std::abs(included_option_detail_sums[excluded_sum.first]) < std::numeric_limits<double>::epsilon()) {
                PLOGD << "Excluding detail: " << excluded_sum.first << "(sum in excluded options: " << excluded_sum.second <<
                         "; sum in included options: " << included_option_detail_sums[excluded_sum.first] << ")";
                excluded_details_.insert(excluded_sum.first);
            } 
        }
        PLOGD << "Excluded detail count: " << excluded_details_.size();
    }

    const bool problem::is_detail_excluded(const std::string detail_name) const {
        return excluded_details_.find(detail_name) != excluded_details_.end();
    }
	
	void problem::linkMathProgram(const MathProgram* MP){ 
		assert(MP->n_var() == original_options().size());
		assert(MP->getConsList().size() == constraints().size()); // no exclusions
		this->preproc_MP = MP;
	}
	void problem::unlinkMathProgram(){
		this->preproc_MP = nullptr;
	}
}
