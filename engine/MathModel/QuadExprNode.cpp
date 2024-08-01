#include "../mathModelling.hpp"

namespace ethelo{
	
using std::vector;

QuadExprNode::QuadExprNode(MathExprNode* expr):
	MathExprNode( MathExprNode::NodeType::QuadExprNode, expr->VM),
	A(expr->VM->n_var(),expr->VM->n_var(), arma::fill::zeros),
	b(expr->VM->n_var(), arma::fill::zeros),
	c{0.0} 
	{	
	assert( expr -> is_quadratic() && !(expr->is_linear()));
	const int n = expr->VM->n_var();
	
	vector<MathExprNode*> args;
	int code;
	
	switch(expr->Type){
		case MathExprNode::NodeType::MultNode:
			args.resize(1);
			args[0] = expr;
			break;
			
		case MathExprNode::NodeType::SumNode:
			expr->decouple(code, args);
			break;
		
		default:
			throw std::runtime_error("Unexpected error in C'tor of QuadExprNode: unknown quadratic type");
	}
	
	LinExp *a1, *a2;
	vector<MathExprNode*> tempList;
	

	for (MathExprNode* term : args){
		switch(term->Type){
			case MathExprNode::NodeType::LinearExp:
				this->add_linear(static_cast<LinExp*>(term));
				delete term;
				continue;
			
			case MathExprNode::NodeType::MultNode:
				term->decouple(code, tempList);
				a1 = static_cast<LinExp*>(tempList.at(0));
				a2 = static_cast<LinExp*>(tempList.at(1));
				
				this->add_product(a1, a2);
				
				for (MathExprNode* tempArg: tempList){
					delete tempArg;
				}
				continue;
			
			default:
				throw std::runtime_error("Unexpected error in C'tor of QuadExprNodes");
		}
	}
}
	
QuadExprNode::QuadExprNode (const QuadExprNode& other):
	MathExprNode{other.Type, other.VM},
	A{other.A}, b{other.b}, c{other.c}
		{}

void QuadExprNode::add_linear(const LinExp* expr){
	b += expr->get_coef();
	c += expr->get_const();
}

void QuadExprNode::add_product(const LinExp* expr1, const LinExp* expr2){
	arma::vec v1{expr1->get_coef()}, v2{expr2->get_coef()};
	double c1{expr1->get_const()}, c2{expr2->get_const()};
	
	A += v1 * v2.t();
	b += c1 * v2 + c2 * v1;
	c += c1 * c2;
	
}

MathExprNode::NodeType QuadExprNode::decouple(int& code, std::vector<MathExprNode*> &args){
	code = 0;
	args.resize(1);
	args[0] = this;
	return MathExprNode::NodeType::QuadExprNode;
}


MathExprNode* QuadExprNode::scale(double k){
	A *= k; b *= k; c *= k;
	return this;
}

void QuadExprNode::save_content(std::ostream& out) const{
	// This structure is only intended for the RLT procedure and not supposed to 
	//    be saved
	throw std::runtime_error{"save_spec for QuadExprNode is not implemented"};
}

bool QuadExprNode::is_similar(const MathExprNode* other) const{
	if (other->getName() != this->getName()){
		return false;
	}
	const QuadExprNode* temp = static_cast<const QuadExprNode*>(other);
	
	return (abs(c - temp->c) <= MathProgram::epsilon) &&
		is_zeros(b - temp->b) &&
		is_zeros(arma::vectorise(A - temp->A));
}

double QuadExprNode::evaluate( const arma::vec& x) const{
	return arma::as_scalar(x.t() * A * x) + arma::dot(b, x) + c ;
} 

AD QuadExprNode::evaluate( ADvector& x) const{
	// only used in linear solvers with RLT reformulation
	// not intended to be used with AD packages
	throw std::runtime_error{"QuadExprNode not supposed to be evaluated with AD"};
} 

}// namespace ethelo