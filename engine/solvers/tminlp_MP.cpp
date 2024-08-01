#include "../ethelo.hpp"
#include "../mathModelling.hpp"

#include "tminlp_MP.hpp"
namespace ethelo {
namespace coin {
	
inline bool feq(const double& a, const double& b){
	return fabs(a-b)<1e-8;
}
	
// ===================  EvalWrapper members ===================


tminlp_MP::EvalWrapper::EvalWrapper( const MathProgram* MP): 
	MP{MP}, eth("EvalWrapper", MP){}

void tminlp_MP::EvalWrapper::evaluate(ADvector& fg, ADvector& x){
	// mimics evaluator::evaluate
	assert(x.size() == MP->n_var());
	const auto& consList = MP->getConsList();
	fg.resize(1 + consList.size());
	
	ADvector temp_ethelo(1);
	eth(x, temp_ethelo);
	fg[0] = temp_ethelo[0];
	
	for (int i=0;i<consList.size();i++){
		fg[i+1] = consList[i].expr->evaluate(x);
	}
	// exclusions are included within consList
}

// ===================  tminlp_MP members ===============
tminlp_MP::tminlp_MP(const MathProgram* MP):
	tminlp_Base(MP), fg_eval_(MP){
		PLOGD << "tminlp_MP constructor";
		const problem& p_ = *(MP->getProblem());
		size_t n = MP->n_var();
		xi_.resize(n);
        xl_.resize(n);
        xu_.resize(n);
        for (int i = 0; i < n; i++) {
            xi_[i] = 0.0; 
            xl_[i] = 0.0;
            xu_[i] = 1.0;
        }
		
		// constraint bounds
		const auto& cons = MP->getConsList();
        size_t m = cons.size();
        gl_.resize(m);
        gu_.resize(m);
        for (int i = 0; i < cons.size(); i++) {
            gl_[i] = cons[i].lb;
            gu_[i] = cons[i].ub;
        }
		
        PLOGD << "TMINLP reset";
        solve_callback_.reset(new CppAD::ipopt::solve_callback<Dvector, ADvector, EvalWrapper>(
            1,
            n,
            m,
            xi_,
            xl_,
            xu_,
            gl_,
            gu_,
            fg_eval_,
            false, // retape
            false, // sparse_forward
            false, // sparse_reverse
            solve_result_
        ));
        PLOGD << "TMINLP constructor end";
	
}

// For general Problems, we'll just let solve_callback_ handle everything

bool tminlp_MP::get_nlp_info(Index& n, Index& m, Index& nnz_jac_g,
								Index& nnz_h_lag, TNLP::IndexStyleEnum& index_style)
{
	
	return solve_callback_->get_nlp_info(n, m, nnz_jac_g, nnz_h_lag, index_style);
	
}

bool tminlp_MP::get_bounds_info(Index n, Number* x_l, Number* x_u,
								   Index m, Number* g_l, Number* g_u)
{
	return solve_callback_->get_bounds_info(n, x_l, x_u, m, g_l, g_u);
}

bool tminlp_MP::get_starting_point(Index n, bool init_x, Number* x,
									  bool init_z, Number* z_L, Number* z_U,
									  Index m, bool init_lambda,
									  Number* lambda)
{
	return solve_callback_->get_starting_point(n, init_x, x, init_z, z_L, z_U, m, init_lambda, lambda);
}

bool tminlp_MP::eval_f(Index n, const Number* x, bool new_x, Number& obj_value)
{
	return solve_callback_->eval_f(n, x, new_x, obj_value);
}

bool tminlp_MP::eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f)
{
	return solve_callback_->eval_grad_f(n, x, new_x, grad_f);
}

bool tminlp_MP::eval_g(Index n, const Number* x, bool new_x, Index m, Number* g)
{
	return solve_callback_->eval_g(n, x, new_x, m, g);
}

bool tminlp_MP::eval_jac_g(Index n, const Number* x, bool new_x,
							  Index m, Index nele_jac, Index* iRow, Index *jCol,
							  Number* values)
{
	return solve_callback_->eval_jac_g(n, x, new_x, m, nele_jac, iRow, jCol, values);
}

bool tminlp_MP::eval_h(Index n, const Number* x, bool new_x,
						  Number obj_factor, Index m, const Number* lambda,
						  bool new_lambda, Index nele_hess, Index* iRow,
						  Index* jCol, Number* values)
{
	return solve_callback_->eval_h(n, x, new_x, obj_factor, m, lambda, new_lambda, nele_hess, iRow, jCol, values);
}

} // namespace coin
} // namespace ethelo
