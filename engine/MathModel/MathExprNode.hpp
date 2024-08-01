#pragma once
#include <armadillo>
#include <string>
#include <utility>
#include "../ADShorthands.hpp"

#define SHOW_Mem_Path 0 //flag for debugging

/* 
  This file defines the base class for Math Expression Tree. All leaf nodes are supposed
    to be Linear expressions (LinExp), unless the expression tree consists of a single node.
	
  In general cases, opearations between nodes should be done via the functions in the end of  
    this file to allow for simpification of the expression tree.
*/
namespace ethelo{
	
class VarMask;

class MathExprNode{

  public:
	enum NodeType {SumNode, MultNode, DivNode, AbsNode, LinearExp, SqrtNode,	// These 6 node types are used in translation in evaluator class
				QuadExprNode				// This is intended to be used in reformulation in MathProgram only
				};

	const NodeType Type;
	const VarMask* VM;
	
  private:
	// save_content is used as subprocess in save() function below,
	// it prints content of a node to out, without node type
	virtual void save_content(std::ostream& out) const = 0;

  protected:
	MathExprNode(NodeType T, const VarMask* VM);
	MathExprNode(MathExprNode& other) = delete; // disable copy constructor
	
  public:
	// Pure Virtual methods
	virtual MathExprNode* scale(double k) = 0; //multiply by constant
	virtual MathExprNode* deepcopy() const = 0;
	
	/* decouple(code, args) destroyes current structure and returns
		its subnodes in args and assigns a status code to code for 
		non-pointer fields. By default the returned status code is 0
	*/
	virtual NodeType decouple(int& code, std::vector<MathExprNode*> &args) = 0;
	
	virtual ~MathExprNode();
	
	/* print(out) prints the MathExprNode in human-readable format,
		evaluate(x) evaluates the expression at given point x,
		save(out) saves the MathExprNode into file via out
		All 3 functions throw exceptions unless being override
	*/
	virtual void print(std::ostream& out) const;
	virtual double evaluate( const arma::vec& x) const = 0;
	virtual AD evaluate( ADvector& x) const = 0;
	void save(std::ostream& out) const;
	
	// predict_bound sets ub/lb to estimated upper/lower bound of expression.
	// By default, it sets lb=-INFTY, ub=INFTY unless being override
	virtual void predict_bound(double& lb, double& ub) const;
	virtual bool is_linear() const{	return false;} // to be override
	virtual bool is_fraction() const{ return false;} //to be override in DivNode
	virtual bool is_quadratic() const{ return (is_linear() || false) ;}
	
	// is_leaf() returns whether the node is a leaf node. It returns 
	//    true for LinExp, QuadExprNode and false otherwise
	virtual bool is_leaf() const;

	// returns type of node in string
	virtual std::string getName() const = 0;
	
	virtual bool is_similar(const MathExprNode* other) const;

};

inline bool is_zeros(const arma::vec& v){
	return arma::all(arma::abs(v) < 10.0 * std::numeric_limits<double>::epsilon());
}

/* The following methods assumes that all MathExprNode* passed in are dynamically 
    allocated, and are not all nullptrs. After the function call, the MathExprNode*
	passed in will be either 1) destroyed, or 2) used as argument of other operator.
	Either way, the passed in address should be treated as invalid after the call. 

	In the case that nullptr is passed in as parameter:
		add, sub:	Treat nullptr as constant 0
		mult, div:	Treat nullptr as constant 1

	All functions below return a pointer to a dynamically allocated address.
*/


MathExprNode* MExprSigma(std::vector<MathExprNode*>& args); // sum all nodes in args
MathExprNode* MExprAdd(MathExprNode* arg1, MathExprNode* arg2);
MathExprNode* MExprSub(MathExprNode* arg1, MathExprNode* arg2);
MathExprNode* MExprMult(MathExprNode* arg1, MathExprNode* arg2);
MathExprNode* MExprDiv(MathExprNode* arg1, MathExprNode* arg2);
MathExprNode* MExprAbs(MathExprNode* arg);
MathExprNode* MExprSqrt(MathExprNode* arg);

/* loadMExprNode(VM, fin) takes NodeType id from fin and calls the corresponding
	static load method to create a Node associated with VM.
	The returned node is dynamically allocated
*/
MathExprNode* loadMExprNode(const VarMask* VM, std::istream& fin);
} // namespace ethelo





