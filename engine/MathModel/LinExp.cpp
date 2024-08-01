#include "../mathModelling.hpp"
#include "../ethelo.hpp"
#include <iostream>


namespace ethelo{

LinExp::LinExp(const VarMask* VM, const arma::vec& a, double b):
	MathExprNode(NodeType::LinearExp,VM), a{a}, b{b}
	{}

LinExp::LinExp(const VarMask* VM, double c):
	MathExprNode(NodeType::LinearExp, VM),
	a{arma::zeros(VM->n_var())},
	b{c}
	{}


void LinExp::operator += (const LinExp& other){
	a += other.get_coef();
	b += other.get_const();
}

MathExprNode::NodeType LinExp::decouple(int& code, std::vector<MathExprNode*> &args){
	code = 0;
	args.clear();
	args.push_back(this);
	return this->Type;
}


MathExprNode* LinExp::scale(double k){
	a *= k;
	b *= k;
	return this;
}

void LinExp::print(std::ostream& out) const{
	out << "\n( ";
	for (int i=0;i<a.n_elem; i++){
		if (a[i] != 0.0){
			out << a[i] << "* x_"<< i << " + ";
		}
	}
	out << b << " )\n";
}
double LinExp::evaluate( const arma::vec& x) const{
	assert(x.n_elem == a.n_elem);
	if (a.n_elem == 0){	return b;}
	return (arma::dot(a,x) + b) ;
}

AD LinExp::evaluate(ADvector& x) const{
	assert(x.size() == a.n_elem);
	AD sum(b);
	for (int i=0;i<a.n_elem;i++){
		if (a[i] == 0.0){ continue;}
		if (a[i] == 1.0){       sum += x[i];}
		else if (a[i] == -1.0){	sum -= x[i];}
		else{                   sum += a[i] * x[i]; }
	}
	return sum;
}

void LinExp::predict_bound(double& lb, double& ub) const{
	lb = b; ub = b;
	double temp1,temp2;
	for (int i=0;i<a.size(); i++){
		temp1 = a[i] * VM->get_lb(i);
		temp2 = a[i] * VM->get_ub(i);

		lb += std::min(temp1, temp2);
		ub += std::max(temp1, temp2);
	}
}


void LinExp::save_content(std::ostream& out) const{
	for ( int i=0; i<a.n_elem; i++){
		out << a[i] << " ";
	}
	out << b << std::endl;
}

LinExp* LinExp::load(const VarMask* VM, std::istream& fin){
	const int n=VM->n_var();
	arma::vec a(n);
	for (int i=0; i<n; i++){
		fin >> a[i];
	}
	double b;
	fin >> b;
	return new LinExp(VM, a, b);
	
}

bool LinExp::is_similar(const MathExprNode* other) const {
	if (other->getName() != this->getName()){
		return false;
	}
	const LinExp* temp = static_cast<const LinExp*>(other);
	if (abs(temp->b - this->b) > MathProgram::epsilon){
		return false;
	}
	if (temp->a.n_elem != this->a.n_elem ){
		return false;
	}
	if (!is_zeros(temp->a - this->a)){
		return false;
	}
	return true;
}

} // namespace ethelo