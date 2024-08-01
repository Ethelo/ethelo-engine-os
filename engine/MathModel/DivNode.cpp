#include "../mathModelling.hpp"


namespace ethelo{
using namespace std;

MathExprNode::NodeType DivNode::decouple(int& code, std::vector<MathExprNode*> &args){

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


void DivNode::save_content(std::ostream& out) const{
	arg1->save(out);
	arg2->save(out);
}


bool DivNode::is_similar(const MathExprNode* other) const{
	if (other->getName() != this->getName()){
		return false;
	}
	const DivNode* temp = static_cast<const DivNode*>(other);
	return arg1->is_similar(temp->arg1) && arg2->is_similar(temp->arg2);
}


MathExprNode* DivNode::scale(double k){
	if (std::abs(k) >= 1.0){
		this->arg1->scale(k);
	}else{
		this->arg2->scale(k);
	}
	return this;
}

void DivNode::print(std::ostream& out) const{
	this -> arg1 -> print(out);
	out << " / ";
	this -> arg2 -> print(out);
}

double DivNode::evaluate( const arma::vec& x) const{
	double num,denum;
	num   = this->arg1 -> evaluate(x);
	denum = this->arg2 -> evaluate(x);
	return (denum == 0.0 ? 0.0 : num / denum);
}

AD DivNode::evaluate(ADvector& x) const{
	AD num = this->arg1->evaluate(x);
	AD denum = this -> arg2 -> evaluate(x);
	return CondExpEq(denum, AD(0.0), AD(0.0), num/denum);
}

bool DivNode::is_fraction() const{
	return (this->arg1->is_linear()) && (this->arg2->is_linear());
}

DivNode* DivNode::load(const VarMask* VM, std::istream& fin){
	MathExprNode *arg1, *arg2;
	arg1 = loadMExprNode(VM, fin);
	arg2 = loadMExprNode(VM, fin);
	return new DivNode(arg1, arg2);
}

} // namespace ethelo
