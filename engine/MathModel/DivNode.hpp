#pragma once
#include "MathExprNode.hpp"

namespace ethelo{

//DivNode is for division, stored expression is (arg1 divided by arg2)
class DivNode: public MathExprNode{
	
	MathExprNode *arg1, *arg2;
	
	DivNode(MathExprNode* arg1, MathExprNode* arg2):
		MathExprNode{NodeType::DivNode, arg1->VM}, 
		arg1{arg1}, arg2{arg2}{}
	
	virtual void save_content(std::ostream& out) const override;
	
	public:
	friend MathExprNode* MExprDiv(MathExprNode* arg1, MathExprNode* arg2);
	const MathExprNode* getArg1() const { return arg1;}
	const MathExprNode* getArg2() const { return arg2;}
	
	virtual NodeType decouple(int& code, std::vector<MathExprNode*> &args) override;
	virtual bool is_similar(const MathExprNode* other) const override;
	virtual MathExprNode* scale(double k) override;
	virtual void print(std::ostream& out) const override;
	virtual double evaluate( const arma::vec& x) const override;
	virtual AD evaluate( ADvector& x) const override;

	virtual bool is_fraction() const override;
	virtual std::string getName() const override{ return "DivNode";}

	virtual MathExprNode* deepcopy() const override{
		return new DivNode{arg1->deepcopy(), arg2->deepcopy()};
	}

	
	static DivNode* load(const VarMask* VM, std::istream& fin);
	
	virtual ~DivNode(){
		delete arg1;
		delete arg2;
	}
};

}
