#pragma once
#include "MathExprNode.hpp"

namespace ethelo{


//Square Root value
class SqrtNode: public MathExprNode{
	MathExprNode* arg;
	bool negated = false;
	
	virtual void save_content(std::ostream& out) const override;
	
  protected:
	SqrtNode(MathExprNode* arg, bool negated = false);

  public:
	
	const MathExprNode* getArg() const {return arg;}
	
	// code is 1 after call if negated is true, and 0 otherwise
	virtual NodeType decouple(int& code, std::vector<MathExprNode*> &args) override;
	virtual MathExprNode* scale(double k) override; //multiply by constant
	virtual void print(std::ostream& out) const override;
	virtual double evaluate( const arma::vec& x) const override;
	virtual AD evaluate( ADvector& x) const override;
	virtual std::string getName() const override{ return "SqrtNode";}
	virtual MathExprNode* deepcopy() const override{
		return new SqrtNode(arg->deepcopy(), negated);
	}
	virtual bool is_similar(const MathExprNode* other) const override;
	
	virtual ~SqrtNode(){
		delete arg;
	}
	
	//In MathExprNode.hpp
	friend MathExprNode* MExprSqrt(MathExprNode* arg);
	static SqrtNode* load(const VarMask* VM, std::istream& fin);
	
};


} // namespace ethelo