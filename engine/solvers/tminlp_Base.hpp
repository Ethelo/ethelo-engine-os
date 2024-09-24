#pragma once

#include <cppad/ipopt/solve_result.hpp>
#include <cppad/ipopt/solve_callback.hpp>

#include "coin-or/BonTMINLP.hpp"
#include "../ethelo.hpp"

// Base class for custom BONMIN interfaces
/*
  This is an abstract class. see documentation of BONMIN::TMINLP:
  (https://www.coin-or.org/Bonmin/Doxygen/html/class_bonmin_1_1_t_m_i_n_l_p.html)
  
  for functions needed to make a concrete class
*/

namespace ethelo {
	class MathProgram;
	
namespace coin {
    using namespace Ipopt;
    using namespace Bonmin;

    class tminlp_Base : public Bonmin::TMINLP
    {
	protected:
		const MathProgram* MP;
        solution solution_;

    public:
        tminlp_Base(const MathProgram* MP);
        virtual ~tminlp_Base() {};

        const solution& result() const { return solution_; }

    protected:
        virtual bool get_variables_types(Index n, VariableType* var_types);
        virtual bool get_variables_linearity(Index n, Ipopt::TNLP::LinearityType* var_types);
        virtual bool get_constraints_linearity(Index m, Ipopt::TNLP::LinearityType* const_types);
        

        virtual void finalize_solution(TMINLP::SolverReturn status,
                                       Index n, const Number* x, Number obj_value);

        virtual const TMINLP::SosInfo* sosConstraints() const;
        virtual const TMINLP::BranchingInfo* branchingInfo() const;
    };
} // namespace coin
} // namespace ethelo

// for ignoring the coin:: prefix
namespace ethelo { typedef coin::tminlp_Base tminlp_Base; }
