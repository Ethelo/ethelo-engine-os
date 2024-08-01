#include "ethelo.hpp"
#include "mathModelling.hpp"
#include <set> // for translate() functions

namespace ethelo
{
    evaluator::evaluator()
        : p_(NULL)//,ethelo_fun_("NULL")
    { bind();}

    evaluator::evaluator(const problem& p, bool relaxable_constraints)
        : p_(&p), relaxable_constraints_(relaxable_constraints)//,ethelo_fun_("atomic ethelo",p)
    { bind(); if (p.influents().size() == 0) throw std::runtime_error("no influent data"); }

    evaluator::evaluator(const evaluator& other)
        : p_(other.p_)//,ethelo_fun_("atomic ethelo other",*other.p_)
    { bind(); }

    evaluator& evaluator::operator=(const evaluator& other)
    { p_ = other.p_; return *this; }

    void evaluator::bind() {
        T_functions_["abs"] = easy_bind(&evaluator::translate_func_abs, this);
		T_functions_["sqrt"] = easy_bind(&evaluator::translate_func_sqrt, this);
        T_aggregates_["sum"] = easy_bind(&evaluator::translate_agg_sum, this);
        T_aggregates_["sum_all"] = easy_bind(&evaluator::translate_agg_sum_all, this);
        T_aggregates_["mean"] = easy_bind(&evaluator::translate_agg_mean, this);
        T_aggregates_["mean_all"] = easy_bind(&evaluator::translate_agg_mean_all, this);
    }


	void evaluator::translate(
			const FixVar_Mask* FVmask,
			std::vector<MathExprNode*>& arr,
			std::vector<std::set<std::string>>& detail_sets) const
		{
			
		if (!valid()) throw std::runtime_error("use of uninitialized evaluator");
		assert(FVmask != nullptr && FVmask->is_simple());
		assert(FVmask->n_var_orig() == p_->dim());
		
		arr.resize(p_->constraints().size() + p_->exclusions().n_rows);
		detail_sets.resize(p_->constraints().size());

		const int n = p_->dim();

		// Set up x vector 
		// ADvector x(n);
		// for (int i=0;i<n;i++){
			// x[i] = FVmask->get_xVec().at(i);
		// }
		const std::vector<double>& x =  FVmask->get_xVec();
		// FVmask->copyX(x);

		// Extract trees for constraints
        for (int i = 0; i < p_->constraints().size(); i++) {
            const auto& cons = p_->constraints()[i];
            bool encountered_blacklisted_detail = false;
			
            arr[i] = translate_expr(Masked_context(*p_, cons, x, FVmask, detail_sets[i]),encountered_blacklisted_detail);
			
            // Test if we encountered a blacklisted detail, and if the constraint is relaxible, we relax the constraint.
            if(encountered_blacklisted_detail && cons.is_relaxable()){
				delete arr[i];
                arr[i] = nullptr;
            }
        }
		
		//int id;
		std::set<std::string> foo;
		
		//Extract Trees for exclusions
        for (int i = 0; i < p_->exclusions().n_rows; i++){			
            arr[p_->constraints().size() + i] = 
				translate_exclusion(
					Masked_context(*p_, expression(), x, FVmask,foo), 
					arma::vectorise(arma::mat(p_->exclusions().row(i))));
							
		}
	}
	
	void evaluator::translate_displays(const FixVar_Mask* FVmask, std::vector<MathExprNode*>& arr) const{
		if (!valid()) throw std::runtime_error("use of uninitialized evaluator");
		assert(FVmask != nullptr && FVmask->is_simple());
		assert(FVmask->n_var_orig() == p_->dim());
		
		arr.resize(p_->displays().size());

		const int n = p_->dim();

		// Set up x vector 
		// ADvector x(n);
		// for (int i=0;i<n;i++){
			// x[i] = FVmask->get_xVec().at(i);
		// }
		const std::vector<double>& x =  FVmask->get_xVec();

		// Extract trees for display values
		
        bool encountered_blacklisted_detail = false;
		std::set<std::string> foo;
        for (int i = 0; i < p_->displays().size(); i++) {
            const auto& expr = p_->displays()[i];
			
            arr[i] = translate_expr(Masked_context(*p_, expr, x, FVmask, foo),encountered_blacklisted_detail);
		
        }
	}

    size_t evaluator::compile_array(const context& ctx, pANTLR3_BASE_TREE node) const
    {
        size_t index = -1;
        auto child = (pANTLR3_BASE_TREE)node->getChild(node, 0);
        switch (child->getType(child))
        {
        case TOK_INTEGER:
            index = expression::to_integer(child);
            break;
        case TOK_ID: {
            auto ivar = ctx.locals.find(expression::to_string(child));
            if (ivar == ctx.locals.end())
                throw semantic_error(ctx.expr, child, "KeyError", "use of undefined local");
            index = ivar->second;
            break;
        }
        case TOK_STRING: {
            index = ctx.p.options().find(expression::to_string(child));
            if (index < 0)
                throw semantic_error(ctx.expr, child, "KeyError", "use of undefined option");
            break;
        }
        }

        if (index >= ctx.p.options().size())
            throw semantic_error(ctx.expr, child, "IndexError", "index out of bounds of variable array");
        return index;
    }

//==========================================================
/*        The below is for translate() function           */
//==========================================================

	LinExp* evaluator::Masked_context::getNode(int i) const{
		assert(i>=0 && i < p.dim());
		arma::vec coef{arma::zeros(p.dim())};
		coef[i] = 1;
		return getNode(coef);
	}

	LinExp* evaluator::Masked_context::getNode(const arma::vec& coef) const{

		arma::vec a{arma::zeros(this->FVmask->n_var())};
		double b = 0.0;
		for (int i=0; i<p.dim(); i++){
			if (this->FVmask->get_mask_id(i) == -1){
				// b += CppAD::Value(x[i]) * coef[i];
				b += x[i] * coef[i];
			}else{
				a[this->FVmask->get_mask_id(i)] = coef[i];
			}
		}
		return new LinExp(this->FVmask, a,b);
	}

	MathExprNode* evaluator::translate_exclusion( const Masked_context& Mctx, arma::vec exclusion) const{
		
        MathExprNode* ans = nullptr;
        for (size_t i = 0; i < exclusion.size(); i++){
            ans = MExprAdd(ans,
				MExprAbs( MExprSub(Mctx.getNode(i), new LinExp(Mctx.FVmask,exclusion[i]))));
		}
        return ans;

	}

	MathExprNode* evaluator::translate_expr( const Masked_context& Mctx, bool& encountered_blacklisted_detail ) const{
		return translate_expr(Mctx, Mctx.expr.ast(), encountered_blacklisted_detail);
	}

	MathExprNode* evaluator::translate_expr( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const{


        switch (node->getType(node))
        {
        case TOK_INEQ:
            return translate_expr(Mctx, (pANTLR3_BASE_TREE)node->getFirstChildWithType(node, TOK_EXPR), encountered_blacklisted_detail);
        case TOK_EXPR:
            return translate_expr(Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 0), encountered_blacklisted_detail);

		case TOK_PLUS:
			return MExprAdd(
				translate_expr( Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 0), encountered_blacklisted_detail),
				translate_expr( Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 1), encountered_blacklisted_detail));
		case TOK_MINUS:
			return MExprSub(
				translate_expr( Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 0), encountered_blacklisted_detail),
				translate_expr( Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 1), encountered_blacklisted_detail));
		case TOK_MUL:
			return MExprMult(
				translate_expr( Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 0), encountered_blacklisted_detail),
				translate_expr( Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 1), encountered_blacklisted_detail));
		case TOK_DIV:
			return MExprDiv(
				translate_expr(Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 0), encountered_blacklisted_detail),
				translate_expr(Mctx, (pANTLR3_BASE_TREE)node->getChild(node, 1), encountered_blacklisted_detail));

        case TOK_TERM:
            return translate_term(Mctx, node, encountered_blacklisted_detail);

        default:
            break;
        }

        return nullptr;
    }

	MathExprNode* evaluator::translate_term( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const{
		double factor = 1.0;
		MathExprNode* temp;
        for (ANTLR3_UINT32 i = 0; i < node->getChildCount(node); i++) {
            auto child = (pANTLR3_BASE_TREE)node->getChild(node, i);
            switch (child->getType(child))
            {
            case TOK_VAR:
                return translate_var(Mctx, child, encountered_blacklisted_detail);
            case TOK_FRAG:
                return translate_frag(Mctx, child, encountered_blacklisted_detail);
            case TOK_DETAIL:
                return translate_detail(Mctx, child, encountered_blacklisted_detail);
            case TOK_PLUS:
                break;
            case TOK_MINUS:
                factor = -1.0; break;
            case TOK_INTEGER:
                return new LinExp(Mctx.FVmask, factor * expression::to_integer(child));
            case TOK_FLOATING:
                return new LinExp(Mctx.FVmask, factor * expression::to_float(child));
            case TOK_FUNC:
                return translate_func(Mctx, child);
            case TOK_AGG:
                return translate_aggregate(Mctx, child, encountered_blacklisted_detail);
            case TOK_FILT:
                return translate_filter(Mctx, child, encountered_blacklisted_detail);
            case TOK_EXPR:
                return translate_expr(Mctx, child, encountered_blacklisted_detail);

            default:
                break;
            }
        }

        return new LinExp(Mctx.FVmask, 0.0);
	}


	MathExprNode* evaluator::translate_var( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const{
		std::string name = expression::to_string((pANTLR3_BASE_TREE)node->getChild(node, 0));
        auto array = (pANTLR3_BASE_TREE)node->getFirstChildWithType(node, TOK_ARRAY);

        if (name != "x")
            throw semantic_error(Mctx.expr, node, "NameError", "use of undefined variable");

        if (array){
            return Mctx.getNode(compile_array(Mctx, array));
        }else {
			arma::vec coef(arma::zeros(p_->dim()));
            for (auto i : Mctx.options){
				coef[i] = 1.0;
			}
            return Mctx.getNode(coef);
        }
    }


	MathExprNode* evaluator::translate_frag( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const{
		std::string name = expression::to_string((pANTLR3_BASE_TREE)node->getChild(node, 0));
        auto fragments = Mctx.p.fragments();
        auto index = fragments.find(name);

        if (index >= 0)
            return translate_expr(Masked_context(Mctx, fragments[index]), encountered_blacklisted_detail);
        else
            throw semantic_error(Mctx.expr, node, "KeyError", "unknown fragment");
	}

	MathExprNode* evaluator::translate_detail(	const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const{
		std::string name = expression::to_string((pANTLR3_BASE_TREE)node->getChild(node, 0));
        auto array = (pANTLR3_BASE_TREE)node->getFirstChildWithType(node, TOK_ARRAY);
        auto options = Mctx.p.options();
		
		Mctx.detail_set.insert(name);
        if(relaxable_constraints_ && Mctx.p.is_detail_excluded(name)){
            encountered_blacklisted_detail = true;
            PLOGD<<"Encountered blacklisted detail: "<<name<<" setting detail contribution to zero";
            return new LinExp(Mctx.FVmask, 0.0);
        }

        if (array){
            return new LinExp(Mctx.FVmask, options[compile_array(Mctx, array)].get_detail(name));
        }else {
			arma::vec coef{arma::zeros(p_->dim())};
            for (auto i : Mctx.options){
				coef[i] = options[i].get_detail(name);
			}
            return Mctx.getNode(coef);
        }
	}

	MathExprNode* evaluator::translate_func( const Masked_context& Mctx, pANTLR3_BASE_TREE node) const{
		std::string name = expression::to_string((pANTLR3_BASE_TREE)node->getChild(node, 0));
        auto ifunc = T_functions_.find(name);
        if (ifunc == T_functions_.end())
            throw semantic_error(Mctx.expr, node, "KeyError", "unknown function");

        std::vector<pANTLR3_BASE_TREE> arguments;
        for (ANTLR3_UINT32 i = 1; i < node->getChildCount(node); i++)
            arguments.push_back((pANTLR3_BASE_TREE)node->getChild(node, i));

        return ifunc->second(Mctx, node, arguments);
	}

	MathExprNode* evaluator::translate_aggregate( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const{
		std::string name = expression::to_string((pANTLR3_BASE_TREE)node->getChild(node, 0));
        auto iagg = T_aggregates_.find(name);
        if (iagg == T_aggregates_.end())
            throw semantic_error(Mctx.expr, node, "KeyError", "unknown aggregate");

        std::vector<size_t> subset;
        auto options = Mctx.p.options();
        std::string variable = expression::to_string((pANTLR3_BASE_TREE)node->getChild(node, 1));
        auto node_detail = (pANTLR3_BASE_TREE)node->getFirstChildWithType(node, TOK_DETAIL);
        auto node_expr = (pANTLR3_BASE_TREE)node->getFirstChildWithType(node, TOK_EXPR);

        if (!node_detail)
            for (auto i : Mctx.options)
                subset.push_back(i);
        else {
            if (node_detail->getFirstChildWithType(node_detail, TOK_ARRAY))
                throw semantic_error(Mctx.expr, node_detail, "TypeError", "expected detail array");

            std::string detail = expression::to_string((pANTLR3_BASE_TREE)node_detail->getChild(node_detail, 0));
            for (auto i : Mctx.options)
                if (std::abs(options[i].get_detail(detail)) > std::numeric_limits<double>::epsilon())
                    subset.push_back(i);
        }

        std::vector<std::tuple<size_t, MathExprNode*>> values;
        for (size_t i : subset) {
            Masked_context agg_Mcontext(Mctx);
            agg_Mcontext.locals[variable] = i;
            values.push_back(std::make_tuple(i, translate_expr(agg_Mcontext, node_expr, encountered_blacklisted_detail)));
        }
        return iagg->second(Mctx, node, values);

	}

	MathExprNode* evaluator::translate_filter( const Masked_context& Mctx, pANTLR3_BASE_TREE node, bool& encountered_blacklisted_detail ) const{
		auto node_detail = (pANTLR3_BASE_TREE)node->getFirstChildWithType(node, TOK_DETAIL);
        auto node_expr = (pANTLR3_BASE_TREE)node->getFirstChildWithType(node, TOK_EXPR);

        if (node_detail->getFirstChildWithType(node_detail, TOK_ARRAY))
            throw semantic_error(Mctx.expr, node_detail, "TypeError", "expected detail array");
        std::string detail = expression::to_string((pANTLR3_BASE_TREE)node_detail->getChild(node_detail, 0));

        Masked_context filter_context(Mctx);
        filter_context.options.clear();
        for (auto i : Mctx.options)
            if (std::abs(Mctx.p.options()[i].get_detail(detail)) > std::numeric_limits<double>::epsilon())
                filter_context.options.insert(i);

        return translate_expr(filter_context, node_expr, encountered_blacklisted_detail);
	}

	MathExprNode* evaluator::translate_func_abs(
		const Masked_context& Mctx,
		pANTLR3_BASE_TREE node,
		const std::vector<pANTLR3_BASE_TREE>& arguments) const{
		if (arguments.size() != 1)
            throw semantic_error(Mctx.expr, node, "TypeError", "abs() expects exactly 1 argument");
        bool encountered_blacklisted_detail = false;

        return MExprAbs(translate_expr(Mctx, arguments[0], encountered_blacklisted_detail));
		}

	MathExprNode* evaluator::translate_func_sqrt(
		const Masked_context& Mctx,
		pANTLR3_BASE_TREE node,
		const std::vector<pANTLR3_BASE_TREE>& arguments) const{
		if (arguments.size() != 1)
            throw semantic_error(Mctx.expr, node, "TypeError", "sqrt() expects exactly 1 argument");
        bool encountered_blacklisted_detail = false;

        return MExprSqrt(translate_expr(Mctx, arguments[0], encountered_blacklisted_detail));
		}

	MathExprNode* evaluator::translate_agg_sum(
		const Masked_context& Mctx,
		pANTLR3_BASE_TREE node,
		const std::vector<std::tuple<size_t, MathExprNode*>>& values) const{
		MathExprNode* temp = nullptr;

        for (size_t i = 0; i < values.size(); i++){
			temp = MExprAdd(temp,
				MExprMult(
					std::get<1>(values[i]),
					Mctx.getNode(std::get<0>(values[i]))));
		}
        return temp;
		}

	MathExprNode* evaluator::translate_agg_sum_all(
		const Masked_context& Mctx,
		pANTLR3_BASE_TREE node,
		const std::vector<std::tuple<size_t, MathExprNode*>>& values) const
		{
		MathExprNode* temp = nullptr;

        for (size_t i = 0; i < values.size(); i++){
			temp = MExprAdd(temp, std::get<1>(values[i]));
		}
        return temp;
		}

	MathExprNode* evaluator::translate_agg_mean(
		const Masked_context& Mctx,
		pANTLR3_BASE_TREE node,
		const std::vector<std::tuple<size_t, MathExprNode*>>& values) const
		{
		MathExprNode* denum = nullptr;
		MathExprNode* sum = translate_agg_sum(Mctx, node, values);


        for (size_t i = 0; i < values.size(); i++){
            denum = MExprAdd(denum, Mctx.getNode(std::get<0>(values[i])));
		}

        return MExprDiv(sum, denum);
		}

	MathExprNode* evaluator::translate_agg_mean_all(
		const Masked_context& Mctx,
		pANTLR3_BASE_TREE node,
		const std::vector<std::tuple<size_t, MathExprNode*>>& values) const
		{
		double n = values.size();

        MathExprNode* sum = translate_agg_sum_all(Mctx, node, values);
		sum->scale(1.0/n);
        return sum;
		}
}
