#include "../mathModelling.hpp"
#include <vector>

namespace ethelo{
	
using std::vector;

// Constructors
SumNode::SumNode(MathExprNode* arg1, MathExprNode* arg2):
	MathExprNode{MathExprNode::NodeType::SumNode, arg1->VM},
	argList(2)
	{
		assert((arg1 != nullptr) && (arg2 != nullptr));
		assert(arg1->VM == arg2->VM);
		if (arg1->is_linear() && !(arg2->is_linear())){
			std::swap(arg1, arg2);
		}
		argList[0] = arg1; argList[1] = arg2;
}

MathExprNode* SumNode::scale(double k){
	for (MathExprNode* arg: this->argList){
		arg->scale(k);
	}

	return this;
}

void SumNode::print(std::ostream& out) const{
	bool is_head = true;
	for (MathExprNode* arg: this->argList){
		if (is_head){ 
			out<< "("; 	is_head = false;
		}else{
			out << " + ";
		}
		arg->print(out);
	}
	out << ")";
	return;
}

double SumNode::evaluate( const arma::vec& x) const{
	double ans = 0.0;
	for (MathExprNode* arg: this->argList){
		ans += arg->evaluate(x);
	}
	return ans;
	
}
AD SumNode::evaluate( ADvector& x) const{
	AD ans = 0.0;
	for (MathExprNode* arg: this->argList){
		ans += arg->evaluate(x);
	}
	return ans;
	
}

bool SumNode::is_quadratic() const {
	for (MathExprNode* arg: this->argList){
		if (!(arg->is_quadratic())) { return false;}
	}
	return true;
}

/*
void SumNode::predict_bound(double& lb, double& ub) const{
	double L1,L2,U1,U2;
	arg1 -> predict_bound(L1,U1);
	arg2 -> predict_bound(L2,U2);
	lb = L1 + L2;
	ub = U1 + U2;
}*/

MathExprNode::NodeType SumNode::decouple(int& code, std::vector<MathExprNode*> &args){
	args.clear();
	std::swap(args, this->argList);
	code = 0;
	delete this;
	return MathExprNode::NodeType::SumNode;
}

MathExprNode* SumNode::deepcopy() const{
	vector<MathExprNode*> tempArgs(this->argList.size());
	for (int i=0; i<argList.size(); i++){
		tempArgs[i] = this->argList[i]->deepcopy();
	}
	
	return new SumNode(std::move(tempArgs));
}

void SumNode::appendTerm(MathExprNode* expr){
	assert(expr != this);
	
	if (expr->Type == MathExprNode::NodeType::LinearExp){
		if (argList[0]->Type == MathExprNode::NodeType::LinearExp){
			*static_cast<LinExp*>(argList[0]) += *static_cast<LinExp*>(expr);
			delete expr;
			return;
		}else{
			argList.insert(argList.begin(), expr);
			return;
		}
	}
	argList.push_back(expr);
}

SumNode::~SumNode(){
		for (MathExprNode* arg: argList){
			delete arg;
		}
	}

void SumNode::save_content(std::ostream& out) const{
	out << argList.size() << std::endl;
	for (const auto arg: argList){
		arg->save(out);
	}
}


SumNode* SumNode::load(const VarMask* VM, std::istream& fin){
	int n;
	fin >> n;
	vector<MathExprNode*> argList(n);
	for (int i=0; i<n; i++){
		argList[i] = loadMExprNode(VM,fin);
	}
	return new SumNode(std::move(argList));
}

bool SumNode::is_similar(const MathExprNode* other) const{
	if (this->getName() != other->getName()){
		return false;
	}
	const SumNode* temp = static_cast<const SumNode*>(other);
	if (this->argList.size() != temp->argList.size()){
		return false;
	}
	for (int i=0; i< argList.size(); i++){
		if (!argList[i]->is_similar(temp->argList[i])){
			return false;
		}
	}
	return true;
}

} // namespace ethelo