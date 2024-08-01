#pragma once

namespace ethelo
{
	class MathExprNode;
	class LinExp;
	class FixVar_Mask;
	
    class evaluator
    {
        const problem* p_;
        bool relaxable_constraints_; // relax constraints for null-voted options
        // atomic_ethelo ethelo_fun_; // Instantiation of atomic ethelo function

        void bind();

    public:
        // typedef CppAD::AD<double> AD;
        // typedef CPPAD_TESTVECTOR(AD) ADvector;
        // typedef std::map<std::string, AD> ADmap;
        // typedef CPPAD_TESTVECTOR(double) Dvector;

        evaluator();
        evaluator(const problem& p, bool relaxable_constraints = true);
        evaluator(const evaluator& other);
        virtual ~evaluator() {};
        evaluator& operator=(const evaluator& other);

        // void operator()(ADvector& fg, ADvector& x);
        // void evaluate(ADvector& fg, ADvector& x);
        // void displays(ADvector& h, ADvector& x) const;

		//After function call, arr[i] points to the tree corresponding to p_->constraints()[i]
		void translate(const FixVar_Mask* FVmask, std::vector<MathExprNode*>& arr,
		std::vector<std::set<std::string>>& detail_sets) const;
		
		void translate_displays(const FixVar_Mask* FVmask, std::vector<MathExprNode*>& arr) const;

        operator bool() const { return valid(); }
        bool valid() const { return p_ != NULL; }

    private:
        struct context {
            context(const context& ctx, const expression& expr)
                : p(ctx.p), expr(expr), x(ctx.x), options(ctx.options), locals(ctx.locals) {}
            context(const problem& p, const expression& expr, const std::vector<double>& x)
                : p(p), expr(expr), x(x) { for(size_t i = 0; i < p.options().size(); i++) options.insert(i); }

            const problem& p;
            const expression& expr;
            // ADvector& x;
			const std::vector<double>& x;
            std::set<size_t> options;
            std::map<std::string, double> locals;
        };

        // typedef AD evaluator_function(const context& ctx, pANTLR3_BASE_TREE node, const std::vector<pANTLR3_BASE_TREE>& arguments);
        // typedef AD evaluator_aggregate(const context& ctx, pANTLR3_BASE_TREE node, const std::vector<std::tuple<size_t, AD>>& values);
        // std::map<std::string, std::function<evaluator_function>> functions_;
        // std::map<std::string, std::function<evaluator_aggregate>> aggregates_;

		/* The grammar used for generating pANTLR3_BASE_TREE is stored at engine/language/expression.g
			"fragment" refers to those with "@" prefix in JSON file, and
			"detail" refers to those with "$" prefix in JSON file
		*/
		
		
        // AD compile_ethelo(const context& ctx);
        // AD compile_exclusion(const context& ctx, arma::vec exclusion) const;
        // AD compile_expr(const context& ctx, bool& encountered_blacklisted_detail ) const;
        // AD compile_expr(const context& ctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        // AD compile_term(const context& ctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        // AD compile_var(const context& ctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        // AD compile_frag(const context& ctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        // AD compile_detail(const context& ctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        // AD compile_func(const context& ctx, pANTLR3_BASE_TREE node) const;
        // AD compile_aggregate(const context& ctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        // AD compile_filter(const context& ctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        size_t compile_array(const context& ctx, pANTLR3_BASE_TREE node) const;

        // AD func_sqrt(const context& ctx, pANTLR3_BASE_TREE node, const std::vector<pANTLR3_BASE_TREE>& arguments) const;
        // AD func_abs(const context& ctx, pANTLR3_BASE_TREE node, const std::vector<pANTLR3_BASE_TREE>& arguments) const;
        // AD agg_sum(const context& ctx, pANTLR3_BASE_TREE node, const std::vector<std::tuple<size_t, AD>>& values) const;
        // AD agg_sum_all(const context& ctx, pANTLR3_BASE_TREE node, const std::vector<std::tuple<size_t, AD>>& values) const;
        // AD agg_mean(const context& ctx, pANTLR3_BASE_TREE node, const std::vector<std::tuple<size_t, AD>>& values) const;
        // AD agg_mean_all(const context& ctx, pANTLR3_BASE_TREE node, const std::vector<std::tuple<size_t, AD>>& values) const;
		
		//==================================
		/* The below is for the translate() function. The are implemented with a similar logic to
			the compile() functions above, so if a bug is found in the above functions please also
			check the corresponding translate() function as well.
		*/

		//==================================
		/* Masked_context is the context structure with placeholders allowed.
			When an index i is specified to be a placeholder, the value in ctx.x[i]
			will not be used.

		  The "placeholders" in this context is identical to the "Variables" in
		    MathExprTree structures. Note that

		*/

		struct Masked_context: public context{
			// VarID stores ID of MathVars. If VarID[i] = -1 then the numerical
			//   value ctx.x[i] will be used.
			//std::vector<int>& VarID;
			//int VarCount;
			
			const FixVar_Mask* FVmask;
			std::set<std::string>& detail_set;
			
			Masked_context(const Masked_context& Mctx)
				: context(Mctx, Mctx.expr), FVmask{Mctx.FVmask}, detail_set{Mctx.detail_set} {};
			
			Masked_context(const Masked_context& Mctx, const expression& expr)
				: context(Mctx, expr), FVmask{Mctx.FVmask}, detail_set{Mctx.detail_set} {};

			//With this constructor x must have same values as FVmask->get_Xvec()
            Masked_context(const problem& p, const expression& expr, const std::vector<double>& x, const FixVar_Mask* FVmask, std::set<std::string>& detail_set)
				: context(p,expr,x), FVmask{FVmask}, detail_set{detail_set}{};

			// Returns a MathExprNode that 1)represents constant ctx.x[i] if VarID[i] = -1, and 2) represents the (VarID[i])-th placeholder.
			LinExp* getNode(int i) const;
			LinExp* getNode(const arma::vec& coef) const;
		};

		typedef MathExprNode* translator_function(const Masked_context& Mctx, pANTLR3_BASE_TREE node, const std::vector<pANTLR3_BASE_TREE>& arguments);
        typedef MathExprNode* translator_aggregate(const Masked_context& Mctx, pANTLR3_BASE_TREE node, const std::vector<std::tuple<size_t, MathExprNode*>>& values);
        std::map<std::string, std::function<translator_function>> T_functions_;
        std::map<std::string, std::function<translator_aggregate>> T_aggregates_;

		//Translation functions
        MathExprNode* translate_exclusion( const Masked_context& Mctx, arma::vec exclusion) const;
        MathExprNode* translate_expr( const Masked_context& Mctx, bool& encountered_blacklisted_detail ) const;

        MathExprNode* translate_expr( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        MathExprNode* translate_term( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        MathExprNode* translate_var( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        MathExprNode* translate_frag( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        MathExprNode* translate_detail(	const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;

        MathExprNode* translate_func( const Masked_context& Mctx, pANTLR3_BASE_TREE node) const;
        MathExprNode* translate_aggregate( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;
        MathExprNode* translate_filter( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const;

        MathExprNode* translate_func_abs(
			const Masked_context& Mctx,
			pANTLR3_BASE_TREE node,
			const std::vector<pANTLR3_BASE_TREE>& arguments) const;
		MathExprNode* translate_func_sqrt(
			const Masked_context& Mctx,
			pANTLR3_BASE_TREE node,
			const std::vector<pANTLR3_BASE_TREE>& arguments) const;
        MathExprNode* translate_agg_sum(
			const Masked_context& Mctx,
			pANTLR3_BASE_TREE node,
			const std::vector<std::tuple<size_t, MathExprNode*>>& values) const;
        MathExprNode* translate_agg_sum_all(
			const Masked_context& Mctx,
			pANTLR3_BASE_TREE node,
			const std::vector<std::tuple<size_t, MathExprNode*>>& values) const;
        MathExprNode* translate_agg_mean(
			const Masked_context& Mctx,
			pANTLR3_BASE_TREE node,
			const std::vector<std::tuple<size_t, MathExprNode*>>& values) const;
        MathExprNode* translate_agg_mean_all(
			const Masked_context& Mctx,
			pANTLR3_BASE_TREE node,
			const std::vector<std::tuple<size_t, MathExprNode*>>& values) const;
    };
}
