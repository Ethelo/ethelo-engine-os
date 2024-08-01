#pragma once
#include <map>
#include <vector>
#include "VarMask.hpp"
#include <utility>

namespace ethelo{
	
/*
	QuadTermID represents a quadratic term. More specifically, (i,j) represents
	  the term x_i * x_j. We ensure that i<j in QuadTermID to avoid double counting.
*/
	
struct QuadTermID{
	int i,j;
	
	QuadTermID(int _i, int _j): 
		i{std::min(_i,_j)}, 
		j{std::max(_i,_j)}
		{}
		
	QuadTermID(const QuadTermID& other):
		i{other.i}, j{other.j} {}
	
	bool operator < (const QuadTermID& other) const{
		if (i != other.i)	{ return i < other.i; }
		else				{ return j < other.j; }
	}
};

/*
	RLT_Mask introduces a new variable for each quadratic term x_i*x_j with i != j,
	  the original variables (x_i's) keep their original index.
	This class is intended to be used for linearizing quadratic constraints, and we
	  require that the original variables are all binary 
*/
class RLT_Mask: public VarMask{
	
	bool dirty;
	std::map<QuadTermID, int> indexMap;		//Map QuadTermID to new index
	std::vector<QuadTermID> inv_indexMap;	//inverse index map, shifted by n_orig_local
	
  protected:
	virtual void local_mask(double* masked_vec, const double* unmasked_vec) const override;
	
	virtual void local_unmask(double* unmasked_vec, const double* masked_vec) const override;
	
	virtual MathExprNode* transform_leaf(MathExprNode* expr) const override;
	
  public:
	RLT_Mask(int n_var);
	RLT_Mask(const RLT_Mask& other);
	
	bool is_clean() const override{ return !dirty;}
	void update(); // updates index maps, set dirty to false
	void reset();  
	const std::map<QuadTermID, int>& get_map() const{ return indexMap;}


	virtual bool embedding_criteria(const VarMask* VM) const override;
	virtual size_t n_var() const override;
	virtual double get_lb(int id) const override{ return 0.0;}
	virtual double get_ub(int id) const override{ return 1.0;}
	virtual VarMask* deepcopy() const override;
	virtual std::string getName() const override { return "RLT_Mask";}
	
	// signal the quadratic terms used in quadExpr, set dirty to true
	void signal_terms(const QuadExprNode* quadExpr);
	
};

} // namespace ethelo