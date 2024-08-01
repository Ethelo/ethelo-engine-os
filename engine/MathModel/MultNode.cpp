#include "../mathModelling.hpp"

namespace ethelo{
using namespace std;

MathExprNode::NodeType MultNode::decouple(int& code, std::vector<MathExprNode*> &args){

	MathExprNode::NodeType T = this->Type;
	code = 0;
	args.resize(2);
	args[0] = this->arg1;
	args[1] = this->arg2;
	this->arg1 = nullptr;
	this->arg2 = nullptr;
	delete this;

	return T;
}


void MultNode::save_content(std::ostream& out) const{
	arg1->save(out);
	arg2->save(out);
}


bool MultNode::is_similar(const MathExprNode* other) const{
	if (other->getName() != this->getName()){
		return false;
	}
	const MultNode* temp = static_cast<const MultNode*>(other);
	return arg1->is_similar(temp->arg1) && arg2->is_similar(temp->arg2);
}

MathExprNode* MultNode::scale(double k){
	this->arg1->scale(k);
	return this;
}

void MultNode::print(std::ostream& out) const{
	this -> arg1 -> print(out);
	out << " * ";
	this -> arg2 -> print(out);
}

double MultNode::evaluate( const arma::vec& x) const{
	return (this->arg1->evaluate(x)) * (this->arg2 -> evaluate(x));
}

AD MultNode::evaluate( ADvector& x) const{
	return (this->arg1->evaluate(x)) * (this->arg2 -> evaluate(x));
}

bool MultNode::is_quadratic() const {
	return (this->arg1->is_linear()) && (this->arg2->is_linear());
}

MultNode* MultNode::load(const VarMask* VM, std::istream& fin){
	MathExprNode *arg1, *arg2;
	arg1 = loadMExprNode(VM, fin);
	arg2 = loadMExprNode(VM, fin);
	return new MultNode(arg1, arg2);
}


} // namespace ethelo

