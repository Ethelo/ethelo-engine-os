#pragma once
#include "MathExprNode.hpp"
#include <vector>

namespace ethelo{
	
class SumNode : public MathExprNode{
	
	virtual void save_content(std::ostream& out) const override;
	std::vector<MathExprNode*> argList;
	
	SumNode(MathExprNode* arg1, MathExprNode* arg2);
	SumNode(std::vector<MathExprNode*>&& args): MathExprNode(NodeType::SumNode, args.at(0)->VM), argList{args} {}
	
	
  public:
	friend MathExprNode* MExprAdd(MathExprNode* arg1, MathExprNode* arg2); 
	friend MathExprNode* MExprSigma(std::vector<MathExprNode*>& args);
	
	virtual MathExprNode* scale(double k) override;
	virtual void print(std::ostream& out) const override;
	virtual double evaluate( const arma::vec& x) const override;
	virtual AD evaluate( ADvector& x) const override;
	virtual bool is_quadratic() const override;
	virtual NodeType decouple(int& code, std::vector<MathExprNode*> &args) override;
	virtual std::string getName() const override{ return "SumNode";}
	
	virtual MathExprNode* deepcopy() const override;
	
	// expr should be treated as invalid after calling appendTerm(expr)
	void appendTerm(MathExprNode* expr);// may destroy expr
	~SumNode();	// for memory management
	
	virtual bool is_similar(const MathExprNode* other) const override;
	
	static SumNode* load(const VarMask* VM, std::istream& fin);
};

} // namespace ethelo