#include "../mathModelling.hpp"

namespace ethelo{

SqrtNode::SqrtNode(MathExprNode* arg, bool negated): 
	MathExprNode(NodeType::SqrtNode, arg->VM), 
	arg{arg},
	negated{negated} {
		assert(arg != nullptr);
	}

MathExprNode::NodeType SqrtNode::decouple(int& code, std::vector<MathExprNode*> &args){

	code = (negated ? 1 : 0);
	args.resize(1);
	args[0] = this->arg;
	this->arg = nullptr;

	delete this;
	return MathExprNode::NodeType::SqrtNode;
}

MathExprNode* SqrtNode::scale(double k) {
	if (k<0){ negated = !negated;}
	k *= k;
	assert(k > 0);
	this->arg-> scale(k);
	return this;
}

void SqrtNode::print(std::ostream& out) const{
	out << (negated? "- Sqrt(": "Sqrt(");
	this->arg->print(out);
	out << ")\n";
}

double SqrtNode::evaluate( const arma::vec& x) const{
		return (negated? -1.0 : 1.0) * std::sqrt(this->arg->evaluate(x));
	}
	
AD SqrtNode::evaluate(ADvector& x) const{
	return (negated? -1.0 : 1.0) * CppAD::sqrt(this->arg->evaluate(x));
	}



void SqrtNode::save_content(std::ostream& out) const{
	out << negated << std::endl;
	arg->save(out);
}

SqrtNode* SqrtNode::load(const VarMask* VM, std::istream& fin){
	bool b;
	fin >> b;
	
	MathExprNode* arg;
	arg = loadMExprNode(VM, fin);
	SqrtNode* node = new SqrtNode{arg, b};
	return node;
	
	
}
bool SqrtNode::is_similar(const MathExprNode* other) const{
	// check node type
	if (other->getName() != this->getName()){
		return false;
	}
	
	const SqrtNode* temp = static_cast<const SqrtNode*>(other);
	if (temp->negated != this->negated){
		return false;
	}
	return arg->is_similar(temp->arg);
}
} // namespace ethelo