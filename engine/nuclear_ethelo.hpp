#pragma once

#include <armadillo>
#include <vector>

namespace ethelo{

class problem;

// specialized class for evaluating ethelo function and its derivatives
// Does not handle masking of variables
class nuclear_ethelo{
	// constant fields;
	const problem* p_;
	arma::vec mu;
	arma::mat Q;
	bool is_linear;
	size_t n,N; // number of options and respondents 
	
	// layer 1 variables
	arma::vec sat;
	double support, dissonance;
	
	// layer 2 variables, for gradient and hessian only
	bool layer_2_updated = false;
	arma::vec Qx;
	double muk, denom,k;
	
	// vectors for testing
	// std::vector<double> mu_vec, sat_vec, Qx_vec,x_cached;
	
	// functions
	void cache_new_x(const arma::vec& x);
	void update_grad_vars(const arma::vec& x);
  public:
	nuclear_ethelo(const problem* p_);
	double eval(const arma::vec& x, bool new_x);
	arma::vec gradient(const arma::vec& x, bool new_x);
	arma::mat hessian(const arma::vec& x, bool new_x);
};
/*
inline void arma2vec(const arma::vec& x, std::vector<double>& vec){
	vec.resize(x.n_elem);
	for (int i=0;i<x.n_elem; i++){vec[i] = x[i];}
}*/
}// namespace ethelo