#include "../mathModelling.hpp"
#include "../ethelo.hpp"
namespace ethelo{
	
FixVar_Mask::FixVar_Mask(const int n_orig):
	VarMask(n_orig),
	orig_x_lb(n_orig, 0.0),
	orig_x_ub(n_orig, 1.0), 
	x_val(n_orig, 0.0), 
	dirty{true},
	BINARY{true},
	ID_orig2m(n_orig, 0), 
	ID_m2orig(n_orig, 0)
	{
		update();
	}
	
FixVar_Mask::FixVar_Mask(const FixVar_Mask& other):
	VarMask(other),
	orig_x_lb{other.orig_x_lb}, 
	orig_x_ub{other.orig_x_ub},
	x_val{other.x_val}, 
	dirty{other.dirty},
	ID_m2orig{other.ID_m2orig},
	ID_orig2m{other.ID_orig2m}
	{
		if (dirty){
			update();
		}
	}
	
bool FixVar_Mask::var_is_fixed(int orig_id) const{
	return ID_orig2m[orig_id] == -1;
}

bool FixVar_Mask::fix_variable(int orig_id, double val){
	
	if (orig_id <0 || orig_id >= x_val.size()){
		//index out of bound
		return false;	
		}
		
	if (var_is_fixed(orig_id)){ 
		//variable already fixed
		return (x_val[orig_id] == val);
		}
	
	
	if (val < orig_x_lb[orig_id] || val > orig_x_ub[orig_id]){
		// Value out of bound
		return false;
	}
	
	dirty = true;
	ID_orig2m[orig_id] = -1;
	x_val[orig_id] = val;
}

bool FixVar_Mask::free_variable(int orig_id){
	if (orig_x_lb[orig_id] == orig_x_ub[orig_id]){
		return false;
	}
	
	dirty = true;
	ID_orig2m[orig_id] = 0;
	x_val[orig_id] = 0.0;
	return true;
}

void FixVar_Mask::update(){
	if (is_clean()){	return;}
	
	int VarCount = 0;
	const int n = this->n_var_orig(0);
	
	ID_m2orig.resize(n);
	ID_orig2m.resize(n);
	
	int pos = 0;
	
	for (int i=0;i < n; i++){
		if (orig_x_lb.at(i) == orig_x_ub.at(i)){
			ID_orig2m[i] = -1;
			x_val[i] = orig_x_lb[i];
		}
		
		if (ID_orig2m.at(i) != -1){
			ID_m2orig[pos] = i;
			ID_orig2m[i] = pos;
			pos ++;
		}
	}
	ID_m2orig.resize(pos);
	dirty = false;
}
	

size_t FixVar_Mask::n_var() const{   
	assert(is_clean());   
	return ID_m2orig.size();         
}

double FixVar_Mask::get_lb(int new_id) const {   
	assert(is_clean());   
	return orig_x_lb[ID_m2orig[new_id]];
}

double FixVar_Mask::get_ub(int new_id) const{   
	assert(is_clean());   
	return orig_x_ub[ID_m2orig[new_id]]; 
}
	   
int FixVar_Mask::get_orig_id(int i) const{  
	assert(is_clean());   
	return ID_m2orig.at(i);             
}

int FixVar_Mask::get_mask_id(int i) const{  
	assert(is_clean());   
	return ID_orig2m.at(i);             
}

bool FixVar_Mask::embedding_criteria(const VarMask* VM) const{
	assert(is_clean());
	
	if (VM->n_var() != this->n_var_orig()){
		return false;
	}
	
	for (int i=0; i<this->n_var(); i++){
		if (VM->get_lb(i) != orig_x_lb.at(i) || 
			VM->get_ub(i) != orig_x_ub.at(i)){
				return false;
			}
	}
	
	return true;
}

void FixVar_Mask::local_mask(double* masked_vec, const double* unmasked_vec) const {
	for (int i=0;i<ID_m2orig.size(); i++){
		masked_vec[i] = unmasked_vec[ID_m2orig[i]];
	}
}

void FixVar_Mask::local_unmask(double* unmasked_vec, const double* masked_vec) const {
	
	for (int i=0; i<x_val.size(); i++){
		unmasked_vec[i] = x_val[i];
	}
	// copyX(unmasked_vec);
	for (int i=0; i<ID_m2orig.size(); i++){
		unmasked_vec[ID_m2orig[i]] = masked_vec[i];
	}
}
	
VarMask* FixVar_Mask::deepcopy() const{
	return new FixVar_Mask(*this);
}


MathExprNode* FixVar_Mask::transform_leaf(MathExprNode* expr) const{
	assert(is_clean());
	MathExprNode::NodeType T = expr->Type;
	if (T == MathExprNode::NodeType::LinearExp){
		LinExp* tempExpr = static_cast<LinExp*>(expr);
		
		arma::vec a_orig = tempExpr->get_coef();
		double b = tempExpr->get_const();
		
		arma::vec a_new(n_var(), arma::fill::zeros);
		double b_new = b;
		for (int i=0; i< n_orig_local; i++){
			if (ID_orig2m[i] == -1){
				// If Var is fixed
				if (x_val[i] != 0.0){
					b_new += a_orig[i] * x_val[i];
				}
			}
			else{
				a_new[get_mask_id(i)] = a_orig[i];
			}
		}
		
		delete tempExpr;
		return new LinExp(this, a_new, b_new);
	}
		
	throw std::runtime_error("FixVar_Mask: Encountered leaf other than LinExp in transform_leaf");
}

bool FixVar_Mask::is_identity() const{
	assert(is_clean());
	return (ID_m2orig.size() == ID_orig2m.size() && prev == nullptr);
}

std::vector<int> FixVar_Mask::makeBridge() const{
	std::vector<int> bridge(ID_orig2m);
	for (int i=0; i<ID_orig2m.size(); i++){
		if (ID_orig2m[i] == -1){
			if (x_val[i] == 0.0){
				bridge[i] = -1;
			}else if (x_val[i] == 1.0){
				bridge[i] = -2;
			}else{
				throw std::runtime_error{"Invalid makeBridge Call"};
			}
		}
	}
	return bridge;
}

int FixVar_Mask::checkBridge(const std::vector<int>& bridge){
	int running_ID = 0;
	for (int i = 0; i<bridge.size() ;i++){
		if (bridge[i] == -1 || bridge[i] == -2){ continue;}
		else if (bridge[i] == running_ID){
			running_ID ++;
		}
		else{
			return -1;
		}
	}
	return running_ID;
}

std::vector<int> FixVar_Mask::makeIdentityBridge(int n){
	std::vector<int> vec(n);
	for (int i=0;i<n;i++){
		vec[i] = i;
	}
	return vec;
}
}// namespace ethelo