#include "ethelo.hpp"
#include "mathModelling.hpp"
#include "nuclear_ethelo.hpp"

using namespace ethelo;

nuclear_ethelo::nuclear_ethelo(const problem* p_):p_{p_}{
	assert(p_ != nullptr);
	
	// Extract the relevant influent matrix and configuration
	const auto& influents = p_->influents();
	const auto& config = p_->config();
	N = influents.n_rows;   // number of respondents
	n = influents.n_cols;   // number of variables

	mu = sum(arma::mat(influents),0).t()/N; // For dense matrices, this might be better
	Q = arma::mat(influents.t()*influents/N - mu*mu.t());
	
	// is ethelo function linear?
	is_linear = (config.collective_identity <= 10.0 * std::numeric_limits<double>::epsilon() || N <= 1);
	
}

void nuclear_ethelo::cache_new_x(const arma::vec& x){
	
	const auto& influents = p_->influents();
    const auto& config = p_->config();
	
	// update satisfaction
	sat = influents * x;
	if (config.per_option_satisfaction) {
		double num_options = 0.0;
		for (int i = 0; i < n; i++)
			num_options += x[i];

		for(int i = 0; i < N; i++) {
			sat[i] = num_options >= 1.0 ? sat[i] / num_options : 0.0;
		}
	}
		
	// support and dissonance
	support = arma::sum(sat) / N;
	dissonance = arma::var(sat,1);
	
	layer_2_updated = false;
}

void nuclear_ethelo::update_grad_vars(const arma::vec& x){
	assert(!layer_2_updated);
	const auto& influents = p_->influents();
    const auto& config = p_->config();
	const double tipping_point = config.tipping_point;
	
	// Qx update 
	Qx = influents.t()*(sat-support)/N;
	
	// k update
	double k_tipping = support;
	double k_not_tipping = support >= 0.0 ? (1.0 - support) : (-1.0 - support);
	
	k = dissonance>= tipping_point ? k_tipping : k_not_tipping;
	muk = (dissonance>=tipping_point ? 1.0 : -1.0);
	
	// denom update
	denom = dissonance>= tipping_point ? 1.0/(1.0 - tipping_point): 1.0/tipping_point;
	
	layer_2_updated = true;
}

double nuclear_ethelo::eval(const arma::vec& x, bool new_x){
	if (new_x){	cache_new_x(x);	}
	double fairness = 0.0;
    const auto& config = p_->config();
	
	if (!is_linear){
		const double& tipping_point = config.tipping_point;
		
		// note that k0 here is defined differently from k in 
		//   gradient and hessian computation
		double k0_tipping = support/(1.0 - tipping_point);
		double k0_not_tipping = support >= 0.0 ? (1.0 - support) / tipping_point : (-1.0 - support) / tipping_point;
		double k0 = dissonance>= tipping_point ? k0_tipping : k0_not_tipping;
		
		fairness = config.collective_identity * (tipping_point - dissonance) * k0;
	}
	
	const double ethelo = fabs(support) >= std::numeric_limits<double>::min() ? support + fairness : 0.0;
	return config.minimize ? ethelo : -ethelo;
}

arma::vec nuclear_ethelo::gradient(const arma::vec& x, bool new_x){	
	if (new_x){ cache_new_x(x);}
	if (!layer_2_updated){ update_grad_vars(x);}
	
    const auto& config = p_->config();
	const auto& tipping_point = config.tipping_point;
	
	arma::vec dfairness(n, arma::fill::zeros);
	if (!is_linear){
		dfairness = config.collective_identity*denom * (-2.0*Qx*k + (tipping_point - dissonance) * mu*muk);
	}
	
	
	const arma::vec grad = mu + dfairness;
	return config.minimize ? grad : -grad;
}

arma::mat nuclear_ethelo::hessian(const arma::vec& x, bool new_x){
	if (new_x){	cache_new_x(x);}
	
	if (is_linear){
		// if ethelo is linear, return zero matrix
		return arma::mat(n,n,arma::fill::zeros);
	}
	
	if (!layer_2_updated){	update_grad_vars(x);}
	
    const auto& config = p_->config();
	
    const arma::mat hess = config.collective_identity*denom * (-2.0*Q*k + 2.0*Qx*mu.t()*muk + 2.0*mu*Qx.t()*muk);
	
	return config.minimize ? hess : -hess;
}