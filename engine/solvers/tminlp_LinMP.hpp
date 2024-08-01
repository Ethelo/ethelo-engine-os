#pragma once

#include "coin/BonTMINLP.hpp"
#include "../nuclear_ethelo.hpp"
#include "tminlp_Base.hpp"

// tminlp (BONMIN) interface for MP with only linear constraints

namespace ethelo {
	class MathProgram;
	
namespace coin {
    using namespace Ipopt;
    using namespace Bonmin;
	
    class tminlp_LinMP : public tminlp_Base
    {
		// structure for rewriting constraints g(x)=Ax+b
		struct lin_cons_set{
			arma::mat A;
			arma::vec b;
			
			// info for nonzeron elements
			int nnz_A;
			std::vector<int> iRow, jCol;
			std::vector<double> nnz_elem;
			
			// constructor, requires all constraints in MP to be linear
			lin_cons_set(const MathProgram* MP);
			
		};
		
		// const MathProgram* MP;
		nuclear_ethelo eth; // ethelo evaluator
		const lin_cons_set fun_g; // constraints evaluator
		const size_t n,m;  // # of free variables, # of constraints
		
		// fields to accomodate fixed variables
		arma::vec x_init;
		std::vector<int> expand_mask;
		
		// cache
		arma::vec x_expanded;
		arma::vec fg_vals;
		
		inline void cache_new_x(int n, const double* x);


    public:
        tminlp_LinMP(const MathProgram* MP);
        virtual ~tminlp_LinMP() {};


    protected:
        virtual bool get_nlp_info(Index& n, Index& m, Index& nnz_jac_g,
                                  Index& nnz_h_lag, TNLP::IndexStyleEnum& index_style);
        virtual bool get_bounds_info(Index n, Number* x_l, Number* x_u,
                                     Index m, Number* g_l, Number* g_u);
        virtual bool get_starting_point(Index n, bool init_x, Number* x,
                                        bool init_z, Number* z_L, Number* z_U,
                                        Index m, bool init_lambda,
                                        Number* lambda);

        virtual bool eval_f(Index n, const Number* x, bool new_x, Number& obj_value);
        virtual bool eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f);
        virtual bool eval_g(Index n, const Number* x, bool new_x, Index m, Number* g);
		
		/* eval_jac_g:
		    The vectors iRow and jCol only need to be set once. The first 
			call is used to set the structure only (iRow and jCol will be 
			non-NULL, and values will be NULL) For subsequent calls, iRow 
			and jCol will be NULL. 
		*/
        virtual bool eval_jac_g(Index n, const Number* x, bool new_x,
                                Index m, Index nele_jac, Index* iRow, Index *jCol,
                                Number* values);
								
		/* eval_h:
		    The vectors iRow and jCol only need to be set once (during the 
			first call). The first call is used to set the structure only 
			(iRow and jCol will be non-NULL, and values will be NULL) For 
			subsequent calls, iRow and jCol will be NULL. This matrix is 
			symmetric - specify the lower diagonal only 
		*/
        virtual bool eval_h(Index n, const Number* x, bool new_x,
                            Number obj_factor, Index m, const Number* lambda,
                            bool new_lambda, Index nele_hess, Index* iRow,
                            Index* jCol, Number* values);
    };
} // namespace coin
} // namespace ethelo

// for ignoring the coin:: prefix
namespace ethelo { typedef coin::tminlp_LinMP tminlp_LinMP; }
