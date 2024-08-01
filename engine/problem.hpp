#pragma once

namespace ethelo
{
	class MathProgram;
    class problem
    {
        indexed_vector<option> options_;
        indexed_vector<fragment> fragments_;
        indexed_vector<constraint> constraints_;
        indexed_vector<display> displays_;
        arma::mat influents_;
        arma::mat exclusions_;
        configuration config_;

        indexed_vector<option> options_in_scope_;
        arma::mat influents_in_scope_;
        arma::mat exclusions_in_scope_;
        std::vector<size_t> original_option_indexes_;
        std::set<std::string> excluded_details_;

        void load(const std::vector<option>& options,
                  const std::vector<fragment>& fragments,
                  const std::vector<constraint>& constraints,
                  const std::vector<display>& displays,
                  const arma::mat& influents,
                  const arma::mat& exclusions);
				  
		const MathProgram* preproc_MP = nullptr; // field for pre-processed program

    public:
        problem();
        problem(const std::vector<option>& options,
                const std::vector<fragment>& fragments,
                const std::vector<constraint>& constraints,
                const std::vector<display>& displays,
                const arma::mat& influents = arma::mat(),
                const arma::mat& exclusions = arma::mat(),
                const configuration& config = {});
        problem(const problem& other);
        problem(problem&& other);
        virtual ~problem() { };
        problem& operator=(const problem& other);

        void load(const arma::mat& influents);
        void load(const std::vector<constraint>& constraints);
        void exclude(const arma::mat& exclusions);
        void exclude(const arma::uvec& exclusions);
        void configure(const configuration& config) { config_ = config; }
        const bool is_detail_excluded(const std::string detail_name) const;

        size_t dim() const { return options().size(); }
        const indexed_vector<option>& options() const { return options_in_scope_.empty() ? options_ : options_in_scope_; }
        const indexed_vector<option>& original_options() const { return options_; }
        const size_t original_option_index(const size_t active_index) const { return options_in_scope_.empty() ? active_index : original_option_indexes_[active_index]; }
        const indexed_vector<fragment>& fragments() const { return fragments_; }
        const indexed_vector<constraint>& constraints() const { return constraints_; }
        const indexed_vector<display>& displays() const { return displays_; }
        const arma::mat& influents() const { return options_in_scope_.empty() ? influents_ : influents_in_scope_; }
        const arma::mat& exclusions() const { return options_in_scope_.empty() ? exclusions_ : exclusions_in_scope_; }
        const configuration& config() const { return config_; }
		
		void linkMathProgram(const MathProgram* MP);
		void unlinkMathProgram();
		const MathProgram* getPreproc_MP() const{ return preproc_MP;}
		
    };
}
