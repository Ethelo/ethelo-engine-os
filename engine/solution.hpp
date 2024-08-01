#pragma once

namespace ethelo
{
    struct solution
    {
        bool success = false;
        std::string status = "unknown";
        arma::vec x;
        arma::vec fgh;
        std::set<std::string> options;
        operator bool() const { return success; }
		
		// completes solution object when solver successfully returned a solution sol
		// sol should be an array of size p.dim()
		void fill_success(const problem& p, const double* sol);
		
		// completes solution object when solver does not return a solution
		void fill_failure(const std::string& status);
		
		// compute_fgh(p,x) computes value of ethelo func, constraints, 
		//   and display values. It is required that problem p has 
		//   preproc_MP attached as it will be used for computation
		static arma::vec compute_fgh(const problem& p, const arma::vec& x);
    };
}
