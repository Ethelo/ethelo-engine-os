#include "ethelo.hpp"
#include "mathModelling.hpp"
#include "atomic_ethelo.hpp"
#include "nuclear_ethelo.hpp"

namespace ethelo{

void solution::fill_success(const problem& p, const double* sol){
	this->success = true;
	this->status = "success";
	
	const int n = p.dim();
	arma::vec vx(p.options().size(), arma::fill::zeros);

	for (int i = 0; i < n; i++) {
		if (sol[i] > 0.5) {
			this->options.insert(p.options()[i].name());
			vx(i) = 1.0;
		}
	}
	this->fgh = solution::compute_fgh(p,vx);

	// map back to original option list (with no option exclusions)
	PLOGD << "Mapping solution with blacklisted options back to original";
	size_t original_options_len = p.original_options().size();
	this->x = arma::vec(original_options_len, arma::fill::zeros);
	for (int i = 0; i < n; i++) {
	  this->x[p.original_option_index(i)] = vx[i];
	}
}

void solution::fill_failure(const std::string& status){
	this->success = false;
	this->status = status;
}

arma::vec solution::compute_fgh(const problem& p, const arma::vec& x){
	assert(p.getPreproc_MP() != nullptr);
	atomic_ethelo eth("solEval atomic ethelo", p);
	const MathProgram* MP = p.getPreproc_MP();
	
	size_t n_cons = p.constraints().size();
	size_t n_excl = p.exclusions().n_rows;
	size_t n_displays = p.displays().size();
	
	arma::vec fgh(1+n_cons + n_excl + n_displays,
				  arma::fill::zeros);
	
	// prepare vector for computations using MathProgram;
	// fill all excluded options with zeros
	arma::vec full_x(p.original_options().size(), arma::fill::zeros);
	for (int i=0;i<x.n_elem;i++){
		full_x[p.original_option_index(i)] = x[i];
	}
	
	// computes ethelo value
	fgh[0] = eth.evaluate(x);
	
	/*
	nuclear_ethelo nuEth(&p);
	assert(nuEth.eval(x,true) == fgh[0]);*/
	
	
	//computes constraint values
	const auto& ConsList = MP->getConsList();
	for (size_t i=0; i<n_cons; i++){
		fgh[i+1] = ConsList[i].expr->evaluate(full_x);
	}
	
	// compute exclusion values
	// note exclusions is not computed with MP, as they are not included
	//   in the preprocessed data
	size_t displacement = 1 + n_cons;
	for (size_t i=0;i<n_excl; i++){
		double& count = fgh[displacement + i];
		count=0.0;
		const auto excl = p.exclusions().row(i);
		for (int j=0; j<x.n_elem; j++){
			count += std::abs(x[j] - excl[j]);
		}
	}
	
	// computes display values
	displacement = 1 + n_cons + n_excl;
	const auto& displayList = MP->getDisplayList();
	for (int i=0; i<p.displays().size(); i++){
		fgh[displacement + i] = displayList[i]->evaluate(full_x);
	}
	
	return fgh;

}

} // namespace ethelo