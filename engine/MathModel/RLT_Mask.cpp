#include "../mathModelling.hpp"

namespace ethelo{
	
RLT_Mask::RLT_Mask(int n_var):
	VarMask(n_var, true),
	dirty{true}
		{}
		
RLT_Mask::RLT_Mask(const RLT_Mask& other):
	VarMask(other),
	dirty{other.dirty},
	indexMap{other.indexMap},
	inv_indexMap{other.inv_indexMap}
		{
			update();
		}
		
void RLT_Mask::update(){
	if (is_clean() ) { return;}
	int pos = 0;
	inv_indexMap.clear();
	
	for (auto& kv_pair : indexMap){
		indexMap.at(kv_pair.first) = n_orig_local + pos;
		inv_indexMap.push_back(kv_pair.first);
		
		pos++;
		
	}
	
	dirty = false;
}

void RLT_Mask::reset(){
	indexMap.clear();
	inv_indexMap.clear();
	dirty = true;
}

void RLT_Mask::local_mask(double* masked_vec, const double* unmasked_vec) const{
	assert (is_clean());
	for (int i=0; i<n_orig_local; i++){
		masked_vec[i] = unmasked_vec[i];
	}
	
	for (int i = n_orig_local; i<n_var(); i++){
		masked_vec[i] = 0.0;
	}
}
	
void RLT_Mask::local_unmask(double* unmasked_vec, const double* masked_vec) const{
	assert (is_clean());
	for (int i=0; i<n_orig_local; i++){
		unmasked_vec[i] = masked_vec[i];
	}
}

MathExprNode* RLT_Mask::transform_leaf(MathExprNode* expr) const{
	assert(is_clean());
	MathExprNode::NodeType T = expr->Type;
	if (T == MathExprNode::NodeType::LinearExp){
		LinExp* tempExpr = static_cast<LinExp*>(expr);
		
		arma::vec a_orig = tempExpr->get_coef();
		double b = tempExpr->get_const();
		
		arma::vec a_new(n_var(), arma::fill::zeros);
		for (int i=0; i< n_orig_local; i++){
			a_new[i] = a_orig[i];
		}
		
		delete tempExpr;
		return new LinExp(this, a_new, b);
	}
	
	if (T == MathExprNode::NodeType::QuadExprNode){
		QuadExprNode* tempExpr = static_cast<QuadExprNode*>(expr);
		const QuadTermID* pos;
		
		arma::vec a(n_var(), arma::fill::zeros);
		
		for (int id=0; id<inv_indexMap.size(); id++){
			pos = &(inv_indexMap[id]);
			a[n_orig_local + id] += tempExpr->get_mat().at(pos->i, pos->j);
			a[n_orig_local + id] += tempExpr->get_mat().at(pos->j, pos->i);
		}
		
		for (int i = 0; i < n_orig_local; i++){
			a[i] += tempExpr->get_mat().at(i,i) + tempExpr->get_vec().at(i);
		}
		
		double b = tempExpr->get_const();
		
		delete tempExpr;
		return new LinExp(this, a, b);
	}
	
	throw std::runtime_error("RLT_Mask: Encountered leaf other than LinExp, QuadExprNode in transform_leaf");
}

bool RLT_Mask::embedding_criteria(const VarMask* VM) const{
	return (VM->is_binary() && VM->n_var() == n_orig_local);
}

size_t RLT_Mask::n_var() const{
	return n_orig_local + inv_indexMap.size();
}

VarMask* RLT_Mask::deepcopy() const{
	return new RLT_Mask(*this);
}

void RLT_Mask::signal_terms(const QuadExprNode* quadExpr){
	const arma::mat& A = quadExpr->get_mat();
	dirty = true;
	
	for (int i=0; i< n_orig_local; i++){
		// ignore diagonal
		for (int j=0; j<n_orig_local; j++){
			if (i == j){ continue;}
			if (A.at(i,j) != 0.0){
				indexMap[QuadTermID(i,j)] = 0; // creates entry in map, if not already exists
			}
		}
	}
}
		
}