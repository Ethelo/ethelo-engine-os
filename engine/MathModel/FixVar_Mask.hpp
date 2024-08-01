#pragma once
#include <cassert>
#include "VarMask.hpp"
#include <vector>

/*
	FixVar_Mask is a VarMask used to fix values of selected variables. The
	  un-fixed variables (ie. "free" variables) will be re-indexed to ensure
	  that new indices are contingent
*/

namespace ethelo{
class problem;
/*
	This mask is for fixing a subset of variables to given values and re-indexing the rest
*/
class FixVar_Mask: public VarMask{
	std::vector<double> orig_x_lb, orig_x_ub;
	std::vector<double> x_val; //stores values of fixed variables, in original index
	std::vector<int> ID_orig2m; // ID of corresponding masked variable, -1 for fixed values.

	//dirty bit indicates whether the structure needs updating
	bool dirty, BINARY;
	std::vector<int> ID_m2orig; // original indices of non-fixed variables
	
  protected:
	virtual MathExprNode* transform_leaf(MathExprNode* expr) const override;

  public:
	FixVar_Mask(const int n_orig);
	FixVar_Mask(const FixVar_Mask& other);

	bool var_is_fixed(int orig_id) const;
	bool fix_variable(int orig_id, double val);
	bool free_variable(int orig_id);
	void update();	// This will set dirty bit to false.

	bool is_clean() const override{ return !dirty;}

	// The following requires is_clean() to be true.
	virtual size_t n_var() const override;
	virtual double get_lb(int new_id) const override;
	virtual double get_ub(int new_id) const override;

	int get_orig_id(int i) const;
	int get_mask_id(int i) const;
	const std::vector<double>& get_xVec() const{ return x_val; }
	
	// makeBridge() returns a vector simular to ID_orig2m, except for fixed 
	//   variables, -1 represents fixed value of 0 and -2 represent value of
	//   1. Throws std::runtime_error if contains fixed values other than 
	//   0.0 and 1.0
	std::vector<int> makeBridge() const; 
	// checkBridge() verifies whether bridge fits the format above; returns
	// number of free variables if it fits, returns -1 otherwise 
	static int checkBridge(const std::vector<int>& bridge);
	
	// makeIdentityBridge(n) returns a bridge that represents a mask 
	//   that consists of n free variables and no fixed variable
	static std::vector<int> makeIdentityBridge(int n);
	
	
	virtual bool embedding_criteria(const VarMask* VM) const override;
	virtual std::string getName() const override{ return "FixVar_Mask";}
	virtual bool is_identity() const override;

	// see VarMask.hpp for discription
	virtual void local_mask(double* masked_vec, const double* unmasked_vec) const override;
	virtual void local_unmask(double* unmasked_vec, const double* masked_vec) const override;
	virtual VarMask* deepcopy() const override;

	


};


}// namespace ethelo