#include "solver_cbc.hpp"
#include "../ethelo.hpp"

#include <stdlib.h>

//CBC packages
#include "coin-or/CoinPragma.hpp"
#include "coin-or/OsiClpSolverInterface.hpp"
#include "coin-or/CoinPackedVector.hpp"
#include "coin-or/CbcModel.hpp"
#include "coin-or/CoinPackedMatrix.hpp"

#include <stdio.h>
#include <iostream>
#include <stdexcept>
#include <memory>

#include "../mathModelling.hpp"

#define CBC_checkImage 0

namespace ethelo{
using std::clock;


solver_CBC::solver_CBC(MathProgram* MP):
	_p{*(MP->getProblem())}{
	assert(MP->is_linearizable());
	
	const auto& infl = _p.influents();
	const int n_active_options = _p.dim();
	
	double raw_grad_f[n_active_options];
	
	arma::vec temp = arma::sum(infl, 0).t() / (infl.n_rows);
	temp *= (_p.config().minimize? 1.0 : -1.0 );
	
	for (int i = 0; i<n_active_options ; i++ ){
		raw_grad_f[i] = temp(i);
	}
	
	sol = formulate(*MP, raw_grad_f, false, MP->getVM()->get_maxDepth() == 2);
}

double* solver_CBC::formulate( MathProgram& MP, const double* raw_grad_f, bool force_linearize, bool included_padding){
	
	if (MP.is_linearizable()){
		MP.linearize(false);
	}
	else if (!force_linearize){
		status = solver_CBC::STATUS::Invalid;
		return nullptr;
	}
	
	
	status = STATUS::Unknown;
	const std::vector<MathProgram::MathCons>& ConsList = MP.getConsList();

	const int n = MP.getVM()->n_var();
	int m=0;

	//count linear constraints
	for (const auto& cons : ConsList){
		if (cons.Type == MathProgram::ConsType::Linear){
			m++;
		}
	}
	
	// setup model
	CoinPackedMatrix CoinM;
	CoinM.setDimensions(m, n);
	double rowlb[m],rowub[m];
	double collb[n],colub[n];


	int rowpos= 0;
	LinExp* temp;

	for (const auto& cons : ConsList){
		if (cons.Type != MathProgram::ConsType::Linear){
			if (force_linearize || cons.Type ==  MathProgram::ConsType::VOID) {
				continue;
			}
			status = solver_CBC::STATUS::Invalid;
			return nullptr;
		}

		assert(cons.expr->Type == MathExprNode::NodeType::LinearExp);
		temp = static_cast<LinExp*>(cons.expr);

		for (int i=0;i<n;i++){
			CoinM.modifyCoefficient(rowpos, i, (temp->get_coef())(i));
		}

		rowlb[rowpos] = cons.lb - temp->get_const();
		rowub[rowpos] = cons.ub - temp->get_const();
		assert(rowlb[rowpos] <= rowub[rowpos]);

		rowpos ++;
	}


	for (int i=0; i<n;i++){
		collb[i] = MP.getVM()->get_lb(i);
		colub[i] = MP.getVM()->get_ub(i);
	}
	
	
	int reverse_depth = MP.getVM()->get_maxDepth() - (included_padding? 1:0) - 1;
	double grad_f[n];
	
	assert(n == MP.getVM()->n_var());
	assert(MP.getVM()->n_var_orig(reverse_depth) == _p.dim());
	MP.getVM()->mask(grad_f, raw_grad_f,reverse_depth);


	OsiClpSolverInterface _model;
	_model.loadProblem(CoinM, collb, colub, grad_f, rowlb, rowub);

	_model.setObjSense(1.0);		//Minimization

	//Specify integal variables
	for (int i=0;i<n;i++){
		_model.setInteger(i);
	}
	
	double* tmpSol = this->solve(_model, MP, reverse_depth);
	return tmpSol;

}

double* solver_CBC::solve(OsiClpSolverInterface& model, const MathProgram& MP,int reverse_depth){
	
	
	CbcModel model_cbc(model);
	//model_cbc.setPrintFrequency(1000);
	model_cbc.setCutoffIncrement(1e-8);
	model_cbc.setLogLevel(0); //mute CBC

	model_cbc.initialSolve();
	model_cbc.branchAndBound();

	if (!model_cbc.isProvenOptimal()){
		// optimal solution not found
		if (model_cbc.isProvenInfeasible()){
			status = STATUS::Infeasible;
		}else if (model_cbc.isContinuousUnbounded()){
			status = STATUS::Unbounded;
		}else if (model_cbc.status() == 1){
			status = STATUS::TLE;
		}else{
			status = STATUS::Unknown;
		}
	
		return nullptr;
	}


	status = STATUS::Success;
	const double* rawSol = model_cbc.bestSolution();

	double* fullsol = new double[_p.dim()];
	
	MP.getVM()->unmask(fullsol, rawSol, reverse_depth);	
	return fullsol;
}

/*	Calls CBC solver to obtain a solution */
solution solver_CBC::get_solution(){
	assert(status != solver_CBC::STATUS::Invalid);
	//Convert sol to solution object
	solution solObj;
	const int n = _p.dim();

	solObj.success = false;
	if (status == solver_CBC::STATUS::Success){
		PLOGD << "CBC: Finalizing successful solution";
		solObj.fill_success(_p, sol);
		/*
		solObj.success = true;
		solObj.status = "success";*/
	}
	else{
		PLOGD << "CBC: Finalizing failed solution";
		/*
		solObj.success = false;
		solObj.x  = arma::vec(n, arma::fill::zeros);*/

		switch (status){
			case solver_CBC::STATUS::Infeasible:
				solObj.fill_failure("infeasible");	
				break;
			case solver_CBC::STATUS::Unbounded:
				solObj.fill_failure("continuous_unbounded");	
				break;
			case solver_CBC::STATUS::TLE:
				solObj.fill_failure("limit_exceeded");	
				break;
			default:
				solObj.fill_failure("unknown");	
				break;
		} // switch case
	} // else

	return solObj;
}

} //ethelo namespace

