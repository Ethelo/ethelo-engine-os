#pragma once
#include "MathExprNode.hpp"
#include <armadillo>
#include <utility>

class Sp_LinExp;
class RLT_Mask;

namespace ethelo{
	
class RLT_Mask;

/*
	QuadExprNode represents the expressions x^T Ax + bx + c, where A may not be symmetric
*/

class QuadExprNode: public MathExprNode{
	virtual void save_content(std::ostream& out) const override; // throws error
	// 
	arma::mat A;	
	arma::vec b;
	double c;
	
  public :
	QuadExprNode (MathExprNode* expr); // Also deletes expr
	QuadExprNode (const QuadExprNode& other);
	
	void add_linear(const LinExp* expr);
	void add_product(const LinExp* expr1, const LinExp* expr2);
	
	const arma::mat& get_mat() const { return A;}
	const arma::vec& get_vec() const { return b;}
	double get_const() const         { return c;}
	
	virtual std::string getName() const override{ return "QuadExprNode";}
	virtual bool is_leaf() const override {return true;}
	
	virtual NodeType decouple(int& code, std::vector<MathExprNode*> &args) override;
	virtual MathExprNode* scale(double k) override; //multiply by constant
	
	virtual MathExprNode* deepcopy() const override{
		return new QuadExprNode(*this);
	}
	virtual bool is_similar(const MathExprNode* other) const override;
	
	virtual double evaluate( const arma::vec& x) const override; 
	virtual AD evaluate( ADvector& x) const override; 
};

} //namespace ethelo
