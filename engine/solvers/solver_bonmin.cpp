#include "../ethelo.hpp"
#include "../mathModelling.hpp"

#include "coin/BonBonminSetup.hpp"
#include "coin/BonCbc.hpp"

#include "solver_bonmin.hpp"
#include "tminlp_MP.hpp"
#include "tminlp_LinMP.hpp"

#include <stdio.h>
#include <iostream>

namespace ethelo
{
    using namespace Ipopt;
    using namespace Bonmin;


    void solver_bonmin::solve(const MathProgram* MP)
    {
        PLOGD << "Abort recording";
        CppAD::AD<double>::abort_recording();

        BonminSetup bonmin;
        PLOGD << "Initialize TMINLP";

		// SmartPtr<tminlp_MP> tminlp = new tminlp_MP(MP);
		
		SmartPtr<tminlp_Base> tminlp;
		if (MP->is_linear()){
			// use linear interface
			tminlp = new tminlp_LinMP(MP);
		}else{
			// use general interface
			tminlp = new tminlp_MP(MP);
		}

        PLOGD << "Configure bonmin";
        bonmin.initializeOptionsAndJournalist();
        bonmin.readOptionsFile();
        bonmin.readOptionsString("algorithm B-BB\n");
        bonmin.readOptionsString("bb_log_level 0\n");
        bonmin.readOptionsString("fp_log_level 0\n");
        bonmin.readOptionsString("lp_log_level 0\n");
        bonmin.readOptionsString("milp_log_level 0\n");
        bonmin.readOptionsString("nlp_log_level 0\n");
        bonmin.readOptionsString("oa_cuts_log_level 0\n");
        bonmin.readOptionsString("oa_log_level 0\n");
        bonmin.readOptionsString("sb yes\n");

        // solution s;
        try {
          PLOGD << "Initialize bonmin";
          bonmin.initialize(tminlp);
		  Bab bb;
		  bb(bonmin);
          PLOGD << "Generate tminlp result";
          s = tminlp->result();
        } catch (...) {
          PLOGD << "Unknown error in bonmin solver";
          s.success = false;
          s.status = "unknown_solver_error";
        }

		// return s;
    }
}
