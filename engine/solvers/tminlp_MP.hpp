#pragma once

#include <cppad/ipopt/solve_result.hpp>
#include <cppad/ipopt/solve_callback.hpp>

#include "coin/BonTMINLP.hpp"
#include "../ADShorthands.hpp"
#include "../atomic_ethelo.hpp"
#include "tminlp_Base.hpp"

// BONMIN Interface for general cases

namespace ethelo {
	class MathProgram;
	
namespace coin {
    using namespace Ipopt;
    using namespace Bonmin;

    class tminlp_MP : public tminlp_Base
    {
		class EvalWrapper{
		public:
			const MathProgram* MP;
			atomic_ethelo eth;
			EvalWrapper(const MathProgram* MP);
			void evaluate(ADvector& fg, ADvector& x);
			void operator()(ADvector& fg, ADvector& x){
				return evaluate(fg,x);
			}
		};
		// const MathProgram* MP;
        Dvector xi_;
        Dvector xl_, xu_;
        Dvector gl_, gu_;
		
        EvalWrapper fg_eval_;
        CppAD::ipopt::solve_result<Dvector> solve_result_;
        std::unique_ptr<CppAD::ipopt::solve_callback<Dvector, ADvector, EvalWrapper>> solve_callback_;
		
        // solution solution_;

    public:
        tminlp_MP(const MathProgram* MP);
        virtual ~tminlp_MP() {};

    protected:
		// BONMIN setup functions
        virtual bool get_nlp_info(Index& n, Index& m, Index& nnz_jac_g,
                                  Index& nnz_h_lag, TNLP::IndexStyleEnum& index_style);
        virtual bool get_bounds_info(Index n, Number* x_l, Number* x_u,
                                     Index m, Number* g_l, Number* g_u);
        virtual bool get_starting_point(Index n, bool init_x, Number* x,
                                        bool init_z, Number* z_L, Number* z_U,
                                        Index m, bool init_lambda,
                                        Number* lambda);
		
		// BONMIN evaluation functions
        virtual bool eval_f(Index n, const Number* x, bool new_x, Number& obj_value);
        virtual bool eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f);
        virtual bool eval_g(Index n, const Number* x, bool new_x, Index m, Number* g);
        virtual bool eval_jac_g(Index n, const Number* x, bool new_x,
                                Index m, Index nele_jac, Index* iRow, Index *jCol,
                                Number* values);
        virtual bool eval_h(Index n, const Number* x, bool new_x,
                            Number obj_factor, Index m, const Number* lambda,
                            bool new_lambda, Index nele_hess, Index* iRow,
                            Index* jCol, Number* values);
    };
} // namespace coin
} // namespace ethelo

// for ignoring the coin:: prefix
namespace ethelo { typedef coin::tminlp_MP tminlp_MP; }
