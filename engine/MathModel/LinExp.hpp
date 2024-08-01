#pragma once
#include "MathExprNode.hpp"
#include <armadillo>

namespace ethelo{

class RLT_Mask;
class QuadExprNode;

//Linear Expression
class LinExp: public MathExprNode{
	// This represents the expression (a^T * x + b) with variable x

	arma::vec a;
	double b;
	
	virtual void save_content(std::ostream& out) const override;

  public:
	LinExp(const VarMask* VM, const arma::vec& a, double b);
	LinExp(const VarMask* VM, double b);

	void operator += (const LinExp& other);

	const arma::vec& get_coef() const{ return a; }
	double get_const() const{ return b; }

	virtual NodeType decouple(int& code, std::vector<MathExprNode*> &args) override;
	virtual MathExprNode* scale(double k) override; //multiply by constant

	virtual void print(std::ostream& out) const override;
	virtual double evaluate( const arma::vec& x) const override;
	virtual AD evaluate( ADvector& x) const override;
	virtual void predict_bound(double& lb, double& ub) const override;
	virtual bool is_linear() const override{	return true; }
	virtual bool is_quadratic() const override{	return true; }
	virtual bool is_leaf() const override{		return true; }
	virtual std::string getName() const override{ return "LinExp";}

	virtual MathExprNode* deepcopy() const override{
		return new LinExp(VM, a, b);
	}
	
	virtual bool is_similar(const MathExprNode* other) const override;
	
	static LinExp* load(const VarMask* VM, std::istream& fin);
};


} // namespace ethelo