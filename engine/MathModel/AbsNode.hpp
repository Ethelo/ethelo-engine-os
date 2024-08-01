#pragma once
#include "MathExprNode.hpp"

namespace ethelo{

//Unary operators
// class UnaryOP: public MathExprNode{
	// protected:
	// MathExprNode* arg;

	// UnaryOP(NodeType T, MathExprNode* arg): MathExprNode{T,arg->VM}, arg{arg} {
	// }

	// virtual ~UnaryOP(){
		// delete arg;
	// }

	// public:
	// const MathExprNode* getArg() const {return arg;}
// };

//Absolute value
class AbsNode: public MathExprNode{
	MathExprNode* arg;
	bool negated = false;
	
	virtual void save_content(std::ostream& out) const override;
	
  protected:
	AbsNode(MathExprNode* arg, bool negated = false);

  public:
	
	const MathExprNode* getArg() const {return arg;}
	
	// code is 1 after call if negated is true, and 0 otherwise
	virtual NodeType decouple(int& code, std::vector<MathExprNode*> &args) override;
	virtual MathExprNode* scale(double k) override; //multiply by constant
	virtual void print(std::ostream& out) const override;
	virtual double evaluate( const arma::vec& x) const override;
	virtual AD evaluate( ADvector& x) const override;
	virtual std::string getName() const override{ return "AbsNode";}
	virtual MathExprNode* deepcopy() const override{
		return new AbsNode(arg->deepcopy(), negated);
	}
	virtual bool is_similar(const MathExprNode* other) const override;
	
	virtual ~AbsNode(){
		delete arg;
	}
	
	//In MathExprNode.hpp
	friend MathExprNode* MExprAbs(MathExprNode* arg);
	static AbsNode* load(const VarMask* VM, std::istream& fin);
	
};


} // namespace ethelo