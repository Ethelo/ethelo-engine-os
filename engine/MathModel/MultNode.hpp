#pragma once
#include "MathExprNode.hpp"

namespace ethelo{

// For multiplication, stored expression is (arg1 multiplies arg2)
class MultNode: public MathExprNode{
	
	MathExprNode *arg1, *arg2;
	
	MultNode(MathExprNode* arg1, MathExprNode* arg2):
		MathExprNode{NodeType::MultNode, arg1->VM}, 
		arg1{arg1}, arg2{arg2}{}

  public:
 	const MathExprNode* getArg1() const { return arg1;}
	const MathExprNode* getArg2() const { return arg2;}
  	virtual NodeType decouple(int& code, std::vector<MathExprNode*> &args) override;
	virtual bool is_similar(const MathExprNode* other) const override;
  	virtual void save_content(std::ostream& out) const override;
	virtual MathExprNode* scale(double k) override;
	virtual double evaluate( const arma::vec& x) const override;
	virtual AD evaluate(ADvector& x) const override;
	virtual void print(std::ostream& out) const override;
	virtual bool is_quadratic() const override;
	virtual std::string getName() const override{ return "MultNode";}

	virtual MathExprNode* deepcopy() const override{
		return new MultNode{arg1->deepcopy(), arg2->deepcopy()};
	}

	friend MathExprNode* MExprMult(MathExprNode* arg1, MathExprNode* arg2);
	static MultNode* load(const VarMask* VM, std::istream& fin);
	
	virtual ~MultNode(){
		delete arg1;
		delete arg2;
	}
};
}

