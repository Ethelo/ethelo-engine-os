#pragma once

#include "../ethelo.hpp"

// #include "ethelo_tminlp.hpp"
#include "coin/OsiClpSolverInterface.hpp"
// #include <cppad/ipopt/solve_result.hpp>
// #include <cppad/ipopt/solve_callback.hpp>

/* The class solver_CBC should only be used when both the ethelo function and
    contraints g(x) are linear or linearizable
*/

namespace ethelo{

class solver_CBC;
class MathProgram;
class FixVar_Mask;

/*	For accessing the protected methods of EtheloTMINLP
	*/
	/*
class ethelo_tminlp_handle: public coin::EtheloTMINLP{
	friend class solver_CBC;
	using coin::EtheloTMINLP::eval_grad_f;
	using coin::EtheloTMINLP::eval_jac_g;

	ethelo_tminlp_handle(const problem& p):coin::EtheloTMINLP{p}{}
};
*/

/*  This class handles the case where both f,g are linear functions
	*/
class solver_CBC{
  public:

	enum STATUS{
		Uninitialized = 0,
		Invalid,		//Program is not linear / linearizable
		Success, 		//Optimal solution found
		Infeasible,
		Unbounded,
		TLE, 			//Time Limit Exceeded
		Unknown
	};

  private:
	const problem &_p;

	// sol is an array of size _p.dim() that stores result of the current
	//  solve. It will be freed on destruction

	STATUS status = STATUS::Uninitialized;
	double* sol = nullptr;

	const double AbsTol = std::numeric_limits<double>::epsilon();
	
	/*formulate(MP, raw_grad_f, force_linearize, included_padding) sets up
	  the interface for calling CBC, and calls the solve() function below.
	  Inputs are:
	    MP: MathProgram to be solved;
		raw_grad_f: coefficients of objective function, including those 
		            for the fixed variables. It should be an array of size
					_p.dim()
		force_linearize: ignores nonlinearizable constraints if true; 
		                 raise exception otherwise
		included_padding: whether MP has an extra layer of VarMask
		                  for fixing inactive options
	*/
	double* formulate(MathProgram& MP, const double* raw_grad_f, bool force_linearize, bool included_padding = false);

	//Solve model with CBC, store status to field status, return (raw) solution given by CBC.
	//  User is responsible for freeing the returned pointer
	//  Returns nullptr if optimal solution not found
	double* solve(OsiClpSolverInterface& model, const MathProgram& MP, int reverse_depth);
	void reset_status(){	status = STATUS::Uninitialized;};

  public:

	// Constructor. The outmost layer of VarMask in MP should be a 
	//   FixVar_Mask fixing 0 or more of the _p.dim() active options
	//   to either 0 or 1
	solver_CBC(MathProgram* MP);
	
	// Destructor
	~solver_CBC(){ delete[] sol;}

	STATUS get_status() const{ return status;}

	solution get_solution();


  protected:
/*
	// Ipopt::Number is just double
	double dist(int size, double* arr1, double* arr2){
		arma::vec V1(arr1, size), V2(arr2, size);
		return arma::norm(V1 - V2, 1);
	}

	bool approx_eq(int size, double* arr1, double* arr2){
		return (dist(size, arr1, arr2) < AbsTol*size);
	}
*/
};
}

