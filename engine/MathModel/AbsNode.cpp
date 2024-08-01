#include "../mathModelling.hpp"

namespace ethelo{

AbsNode::AbsNode(MathExprNode* arg, bool negated): 
	MathExprNode(NodeType::AbsNode, arg->VM), 
	arg{arg},
	negated{negated} {
		assert(arg != nullptr);
	}

MathExprNode::NodeType AbsNode::decouple(int& code, std::vector<MathExprNode*> &args){

	code = (negated ? 1 : 0);
	args.resize(1);
	args[0] = this->arg;
	this->arg = nullptr;

	delete this;
	return MathExprNode::NodeType::AbsNode;
}

MathExprNode* AbsNode::scale(double k) {
	if (k<0){ 
		negated = !negated;
		k = -k;
		}
	this->arg-> scale(k);
	return this;
}

void AbsNode::print(std::ostream& out) const{
	out << (negated? "- Abs(": "Abs(");
	this->arg->print(out);
	out << ")\n";
}

double AbsNode::evaluate( const arma::vec& x) const{
		return (negated? -1.0 : 1.0) * std::abs(this->arg->evaluate(x));
	}
	
AD AbsNode::evaluate(ADvector& x) const{
	return CppAD::abs(this->arg->evaluate(x)) * (negated? -1.0: 1.0);
}

/*void AbsNode::predict_bound(double& lb, double& ub) const{
	double temp_lb, temp_ub;
	arg->predict_bound(temp_lb, temp_ub);

	if (temp_lb < 0.0 && temp_ub > 0.0){
		lb = 0.0;
		ub = std::max(std::abs(temp_lb), std::abs(temp_ub));
	}else{
		lb = std::min(std::abs(temp_lb), std::abs(temp_ub));
		ub = std::max(std::abs(temp_lb), std::abs(temp_ub));
	}

	if (negated){
		std::swap(lb,ub);
		lb *= -1; ub *= -1;
	}
}*/


void AbsNode::save_content(std::ostream& out) const{
	out << negated << std::endl;
	arg->save(out);
}

AbsNode* AbsNode::load(const VarMask* VM, std::istream& fin){
	bool b;
	fin >> b;
	
	MathExprNode* arg;
	arg = loadMExprNode(VM, fin);
	AbsNode* node = new AbsNode{arg, b};
	return node;
	
	
}
bool AbsNode::is_similar(const MathExprNode* other) const{
	if (other->getName() != this->getName()){
		return false;
	}
	const AbsNode* temp = static_cast<const AbsNode*>(other);
	if (temp->negated != this->negated){
		return false;
	}
	return arg->is_similar(temp->arg);
}
} // namespace ethelo