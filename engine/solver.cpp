#include "ethelo.hpp"
#include "solver.hpp"

#include "mathModelling.hpp"
#include "solvers/solver_bonmin.hpp"
#include "solvers/solver_cbc.hpp"

#include <stdexcept>
#include "stopwatch.hpp"

namespace ethelo{


FixVar_Mask solver::formFVMask(const problem& p){
	FixVar_Mask FV(p.dim());
	
	auto &opts = p.options();
	auto &infl = p.influents();
	const int n = p.dim();
	const int n_votes=infl.n_rows;
	
	// first fix variables for determiative options
	if(infl.n_rows == 1){
            // Single respondent/influent case
            for (int i = 0; i < n; i++) {
                if(opts[i].determine()){ // option has determiative flag
                    FV.fix_variable(i, infl(i)>0 ? 1.0 : 0.0);
                }
            }
        }
        else {
            // Group result
            for (int i = 0; i < n; i++) {
                if(opts[i].determine()){ 
					FV.fix_variable(i,1.0);
                }
            }
        }
		
	// next fix auto-balance (if used) to 1.0
	
	int id_AB =	p.options().find("auto-balance");
	if (id_AB >= 0){
		if (FV.var_is_fixed(id_AB)){
			FV.free_variable(id_AB);
		}
		FV.fix_variable(id_AB, 1.0);
	}
	FV.update();
	return FV;
}

MathProgram* solver::formMP(const problem& p){
	FixVar_Mask FV = formFVMask(p); // mask for fixing active options
	
	MathProgram* MP = nullptr;
	
	if (p.getPreproc_MP() == nullptr){
		// if preprocessed data not available
		// this only happens in some legacy testcases
		
		MP = new MathProgram(FV, p, true, true);
		MP->signalBridge(FV.makeBridge());
		return MP;
	}
	
	// now assume preprocessed data available
	assert(p.getPreproc_MP() != nullptr);
	
	// create extra mask FV1 for fixing inactive options
	FixVar_Mask* FV1= new FixVar_Mask(p.original_options().size());
	

	// fix all options to 0 (ie. excluded)
	for (int i=0; i<p.original_options().size(); i++){
		FV1->fix_variable(i, 0.0); 
	}
	// then free the used ones
	for (int i=0; i<p.dim(); i++){
		FV1->free_variable(p.original_option_index(i));
	}
	FV1->update();


	FV.addToFront(FV1); // apply FV1 before FV
	MP = p.getPreproc_MP() -> createImage(FV);
	MP->signalBridge(FV.makeBridge());
	return MP;

	// delete FV1; // FV1 will be deleted in Destructor of FV2
}


solution solver::solve(const problem& p){
	MathProgram* MP = formMP(p);
	assert(MP->hasBridge());
	
	// constants for future reference
	const auto& infl = p.influents();
	const bool ethelo_is_linear = (infl.n_rows == 1) || (p.config().collective_identity <= 10.0 * std::numeric_limits<double>::epsilon());
	
	// flag for solver choice
	const bool useCBC = ethelo_is_linear && MP->is_linearizable();
	// const bool useCBC = false;
	
	if (useCBC){
		solver_CBC cbc(MP);
		delete MP;
		return cbc.get_solution();
	}else{
		// use bonmin
		solver_bonmin bonsolve;
		
		MP->linearize(true); // easy linearization for fractions
		bonsolve.solve(MP);
		delete MP;
		return bonsolve.s;
	}
}

} // namespace ethelo
