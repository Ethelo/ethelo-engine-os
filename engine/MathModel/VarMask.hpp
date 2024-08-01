#pragma once
#include <string>

/*
	VarMask is a structure that is designed to allow change of variables
*/

namespace ethelo{
	
class MathExprNode;

class VarMask{
  protected:
  
	VarMask* prev= nullptr;
	const int n_orig_local; // # of original variables if inner=nullptr
	bool BINARY = true; // whether the variables after substitution are all binary
	
	// subprocedures for mask(), unmask()
	virtual void local_mask(double* masked_vec, const double* unmasked_vec) const = 0;
	virtual void local_unmask(double* unmasked_vec, const double* masked_vec) const = 0;
	
	// Transforms a leaf node (LinExp, QuadExp) into a new one by applying this VM
	// Used as sub-process in transform()
	virtual MathExprNode* transform_leaf(MathExprNode* expr) const =0;
	

	VarMask(int n_orig, VarMask* prev); // prev != nullptr, prev masks are shallow-copied
	VarMask(int n_orig, bool BINARY = true); // sets prev = nullptr
	VarMask(const VarMask& other); 	// prev masks are deep-copied to avoid double free
	
  public:	
	
	virtual ~VarMask(){ delete prev;}
	
	// embedding_criteria(VM) dictates whether VM can be added in front of current mask
	virtual bool embedding_criteria(const VarMask* VM) const =0; 
	
	/* n_var() gives number of variables after applying masks
	   n_var_orig(depth) gives number of original variables before applying
		the last (depth) layers of masks in the list. depth <0 is same as 
		applying all masks
	*/
	virtual size_t n_var() const = 0;			
	int n_var_orig(int depth = -1) const;	 
	
	// get bounds, id in new indexing
	virtual double get_lb(int id) const = 0;
	virtual double get_ub(int id) const = 0;
	
	virtual VarMask* deepcopy() const = 0;
	virtual std::string getName() const = 0;
	bool is_binary() const{ return BINARY; }
	virtual bool is_identity() const{ return false;}
	virtual bool is_clean() const = 0;
	
	// detech_front(depth) keeps last (depth) layers of mask and delete the rest
	VarMask* detech_front(int depth); 
	
	void addToFront(VarMask* VM);
	int get_maxDepth() const;
	const VarMask* get_inner(int layers) const;
	
	// Tanslates vector of coefficients from orignal var. to new var.
	void mask(double* masked_vec, const double* unmasked_vec,int depth = -1) const;
	
	// Translates points (vectors) in new variables back to original variables
	void unmask(double* unmasked_vec, const double* masked_vec,int depth = -1) const;
	
	/* Applies a VarMask on a MathExprNode to get a equivalent MathExprNode
		with semantically different variables
	   Requires: prev == nullptr
	   Caveat  : input address expr will be invalid/deleted after the call
	*/
	MathExprNode* transform(MathExprNode* expr) const;
	
	bool is_simple() const { return prev == nullptr; }
	
	
};
	
}