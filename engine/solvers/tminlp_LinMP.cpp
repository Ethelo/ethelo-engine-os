#include "tminlp_LinMP.hpp"
#include "../mathModelling.hpp"
#include "../problem.hpp"

using namespace ethelo;
using namespace coin;
using namespace Ipopt;
using namespace Bonmin;

// ====== lin_cons_set functions ===============

// Constructor
tminlp_LinMP::lin_cons_set::lin_cons_set(const MathProgram* MP){
	assert(MP->is_linear());
	
	const auto& consList = MP->getConsList();
	const int n = MP->n_var();
	const int m = consList.size();
	
	// extract A,b;
	A.resize(m,n);
	b.resize(m);
	for (int i=0;i<m;i++){
		const LinExp* expr = static_cast<LinExp*>(consList[i].expr);
		A.row(i) = expr->get_coef().t();
		b.at(i) = expr->get_const();
		
	}
	// extract information about jac_g
	iRow.clear();
	jCol.clear();
	nnz_elem.clear();
	for (int i=0;i<m;i++){
		for (int j=0;j<n;j++){
			if (A.at(i,j) == 0.0){ continue;}
			iRow.push_back(i);
			jCol.push_back(j);
			nnz_elem.push_back(A.at(i,j));
		}
	}
	
	iRow.shrink_to_fit();
	jCol.shrink_to_fit();
	nnz_elem.shrink_to_fit();
	
	nnz_A = nnz_elem.size();
	assert(iRow.size() == nnz_A);
	assert(jCol.size() == nnz_A);
}


// ======= tminlp_LinMP functions ============

tminlp_LinMP::tminlp_LinMP(const MathProgram* MP):
	tminlp_Base(MP), eth(MP->getProblem()), fun_g(MP),
	n{MP->n_var()}, m{MP->getConsList().size()}
{
	// linearity check is in fun_g constructor
	
	// handles fixed variables
	const int expanded_dim = MP->getProblem()->dim();
	
	expand_mask.resize(n);
	if (!MP->hasBridge()){
		// no fixed variables; expand_mask set to identity
		x_init = arma::vec(expanded_dim, arma::fill::zeros);
		for (int i=0;i<n;i++){
			expand_mask[i]=i;
		}
	}else{
		const auto& bridge = MP->getBridge();
		assert(expanded_dim == bridge.size());
		x_init = arma::vec(expanded_dim, arma::fill::zeros);
		int pos = 0;
		for (int i=0;i<expanded_dim;i++){
			// check FixVar_Mask::makeBridge for interpretation of bridge vector
			if ( bridge[i] == -1){
				x_init[i] = 0.0;
			}else if ( bridge[i] == -2){
				x_init[i] = 1.0;
			}else{
				expand_mask[pos] = i; 
				pos ++;
			}
		}
		assert(pos == n);
	}
}

void tminlp_LinMP::cache_new_x(int n, const double* x){
	arma::vec x_vec(n);
	x_expanded = x_init;
	for (int i=0;i<n;i++){ 
		x_vec[i] = x[i];
		x_expanded[expand_mask[i]] = x[i];
	}
	
	fg_vals = arma::zeros(m+1);
	fg_vals[0] = eth.eval(x_expanded, true);
	const arma::vec g_val = fun_g.A * x_vec + fun_g.b;
	for (int i=0; i<m; i++){
		fg_vals[i+1] = g_val[i];
	}
}

// =============== BONMIN functions for setup ==================

bool tminlp_LinMP::get_nlp_info(Index& n, Index& m, Index& nnz_jac_g,
                                  Index& nnz_h_lag, TNLP::IndexStyleEnum& index_style){
	n = MP->n_var();
	m = MP->getConsList().size();
	nnz_jac_g = fun_g.nnz_A;
	nnz_h_lag = n * (n+1) / 2; // assumes dense hessian for ethelo
	index_style = TNLP::C_STYLE;
	
	return true;
}
								  
								  
bool tminlp_LinMP::get_bounds_info(Index n, Number* x_l, Number* x_u,
       Index m, Number* g_l, Number* g_u){
	assert(n == MP->n_var());
	assert(m == MP->getConsList().size());
	for (int i=0;i<n;i++){
		x_l[i] = 0.0; 
		x_u[i] = 1.0; // all variables are binary
	}
	
	// extract constraint bounds
	const auto& consList = MP->getConsList();
	for (int j=0;j<m;j++){
		g_l[j] = consList[j].lb;
		g_u[j] = consList[j].ub;
	}
	
	return true;
}	 
							 
bool tminlp_LinMP::get_starting_point(Index n, bool init_x, Number* x,
								bool init_z, Number* z_L, Number* z_U,
								Index m, bool init_lambda,
								Number* lambda)
{	assert( n == MP->n_var());
	assert(init_x);
	assert( ! init_z);
	assert( ! init_lambda);
	
	// initialize with point zero; this point is arbitrarily chosen
	for (int i=0;i<n; i++){
		x[i] = 0.0; 
	}
	return true;
}
	
// ================ BONMIN Evaluation functions =============


bool tminlp_LinMP::eval_f(Index n, const Number* x, bool new_x, Number& obj_value)
{
	if (new_x){ cache_new_x(n,x);}
	
	obj_value = fg_vals[0];
	return true;
}

bool tminlp_LinMP::eval_grad_f(Index n, const Number* x, bool new_x, Number* grad_f)
{
	if (new_x){ cache_new_x(n,x);}
	
	const arma::vec grad_vec = eth.gradient(x_expanded, false);// eth updated in cache_new_x
	
	// Omit partials wrt. fixed variables
	for (int i=0;i<n;i++){
		grad_f[i] = grad_vec[expand_mask[i]];
	}
	return true;
}

bool tminlp_LinMP::eval_g(Index n, const Number* x, bool new_x, Index m, Number* g)
{
	if (new_x){ cache_new_x(n,x);}
	
	for (int i=0;i<m;i++){ 
		g[i] = fg_vals[i+1]; 
	}
	
	return true;
}

bool tminlp_LinMP::eval_jac_g(Index n, const Number* x, bool new_x,
							  Index m, Index nele_jac, Index* iRow, Index *jCol,
							  Number* values)
{
	if (new_x){ cache_new_x(n,x);}
	
	assert(nele_jac == fun_g.nnz_A);
	assert(n == fun_g.A.n_cols);
	assert(m == fun_g.A.n_rows);
	
	if (values == nullptr){
		// extracts positions of nonzero elements
		for (int pos =0; pos < fun_g.nnz_A; pos++){
			iRow[pos] = fun_g.iRow[pos];
			jCol[pos] = fun_g.jCol[pos];
		}
		return true;
	}else{
		// copies jacobian values
		for (int pos =0; pos < fun_g.nnz_A; pos++){
			values[pos] = fun_g.nnz_elem[pos];
		}
		return true;
	}
	// code should not reach here
	throw std::runtime_error("Unknown error");
}

bool tminlp_LinMP::eval_h(Index n, const Number* x, bool new_x,
						  Number obj_factor, Index m, const Number* lambda,
						  bool new_lambda, Index nele_hess, Index* iRow,
						  Index* jCol, Number* values)
{
	if (new_x){ cache_new_x(n,x);}
	
	assert(nele_hess == n * (n+1) / 2);
	
	int pos = 0;
	if ( values == NULL){
		// set positions
		for (int i=0;i<n;i++){
			for (int j=0; j <= i;j++){
				iRow[pos] = i; 
				jCol[pos] = j;
				pos ++;
			}
		}
		assert(pos == nele_hess);
		return true;
	}
	else{
		// compute hessian
		// safely ignore lambdas as all constraints are linear
	
		const arma::mat hess = obj_factor * eth.hessian(x_expanded, false); // eth updated in cache_new_x
		
		// make sure to use same order of traversial as setting positions
		// Omit partials wrt. fixed variables
		for (int i=0;i<n;i++){
			for (int j=0; j <= i;j++){
				values[pos] = hess.at(expand_mask[i],expand_mask[j]);
				pos ++;
			}
		}
		assert(pos == nele_hess);
		return true;
	}
}