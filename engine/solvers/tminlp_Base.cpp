#include "../ethelo.hpp"
#include "../mathModelling.hpp"

#include "tminlp_Base.hpp"
namespace ethelo {
namespace coin {
	

tminlp_Base::tminlp_Base(const MathProgram* MP):
	MP{MP}{
		assert(MP != nullptr);
	}

bool tminlp_Base::get_variables_types(Index n, VariableType* var_types)
{
	// all variables are binary
	for (int i = 0; i <n; i++)
		var_types[i] = BINARY;
	return true;
}

bool tminlp_Base::get_variables_linearity(Index n, Ipopt::TNLP::LinearityType* var_types)
{
	return false;
}

bool tminlp_Base::get_constraints_linearity(Index m, Ipopt::TNLP::LinearityType* const_types)
{
	// return false;
	
	const auto& consList = MP->getConsList();
	for (int i=0; i<m; i++){
		const_types[i] = (consList[i].isLinear()? 
							TNLP::LINEAR : 
							TNLP::NON_LINEAR);
	}
	return true;
}


void tminlp_Base::finalize_solution(TMINLP::SolverReturn status,
									 Index n, const Number* x, Number obj_value)
{
	if (status == SUCCESS) {
		PLOGD << "Finalizing successful solution";
		const problem* p = MP->getProblem();
		if (!(MP->hasBridge())){
			solution_.fill_success(*p, x);
		}
		else{
			const int n = p->dim();
			double x_expanded[p->dim()];
			const auto& bridge = MP->getBridge();
			for (int i=0;i<n;i++){
				// see FixVar_Mask::makeBridge() for meaning of code
				switch (bridge[i]){
					case -1: x_expanded[i] = 0.0; break;
					case -2: x_expanded[i] = 1.0; break;
					default: x_expanded[i] = x[bridge[i]];
				}
			}
			solution_.fill_success(*p, x_expanded);
		}
	} else {
		PLOGD << "Finalizing failed solution";
		// solution_.success = false;

		switch(status)
		{
		case INFEASIBLE:
			solution_.fill_failure("infeasible"); 
			break;
		case CONTINUOUS_UNBOUNDED:
			solution_.fill_failure("continuous_unbounded"); 
			break;
		case LIMIT_EXCEEDED:
			solution_.fill_failure("limit_exceeded"); 
			break;
		case USER_INTERRUPT:
			solution_.fill_failure("user_interrupt"); 
			break;
		case MINLP_ERROR:
			solution_.fill_failure("minlp_error"); 
			break;

		default:
			solution_.fill_failure("unknown"); break;
		}
	}
	PLOGD << "Finalize complete";
}

const TMINLP::SosInfo* tminlp_Base::sosConstraints() const
{
	return NULL;
}

const TMINLP::BranchingInfo* tminlp_Base::branchingInfo() const
{
	return NULL;
}
}
}
