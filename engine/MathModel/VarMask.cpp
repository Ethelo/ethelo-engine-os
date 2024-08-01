#include "../mathModelling.hpp"

namespace ethelo{

VarMask::VarMask(int n_orig, VarMask* prev): 
		n_orig_local{n_orig}, 
		BINARY{prev->is_binary()}, 
		prev{prev} {}
		
VarMask::VarMask(int n_orig, bool BINARY):
	n_orig_local{n_orig}, BINARY{BINARY}, prev{nullptr}
		{}

VarMask::VarMask(const VarMask& other):
	n_orig_local{other.n_orig_local}, 
	BINARY{other.BINARY},
	prev{other.prev==nullptr? nullptr: other.prev->deepcopy()} 
	{
	// if 	(other.prev == nullptr || other.is_identity()){
		// prev = nullptr;
	// }
	// else{
		// prev = other.prev->deepcopy();
	// }
}

void VarMask::mask(double* masked_vec, const double* unmasked_vec, int depth) const{
	if (prev == nullptr || depth == 0){
		return this->local_mask(masked_vec, unmasked_vec);
	}
	else{
		// To get the new variables, apply prev mask first
		double temp[prev->n_var()];
		prev->mask(temp, unmasked_vec, depth-1);
		
		this->local_mask(masked_vec, temp);
	}
}

void VarMask::unmask(double* unmasked_vec, const double* masked_vec, int depth) const{
	if (prev == nullptr || depth == 0){
		return this->local_unmask(unmasked_vec, masked_vec);
	}
	else{
		//To retrieve old variables, undo outter mask first
		double temp[n_orig_local];
		this->local_unmask(temp, masked_vec);
		
		prev->unmask(unmasked_vec, temp, depth-1);
		
	}
}

int VarMask::n_var_orig(int depth) const{
	if (prev == nullptr || depth == 0){
		return n_orig_local;
	}else{
		return prev->n_var_orig(depth-1);
	}
	// return (prev == nullptr? n_orig_local : prev->n_var_orig());
}

void VarMask::addToFront(VarMask* VM){
	if (VM == nullptr){ return; }
	if (prev != nullptr){ return prev->addToFront(VM);}
	
	// general case
	if (!embedding_criteria(VM)){
		throw std::invalid_argument("VarMask: Embedding Criteria not satisfied");
	}
	// if (VM->is_identity()){
		// delete VM;
	// }
	// else{
		// prev = VM; 
	// }
	prev = VM;
}

VarMask* VarMask::detech_front(int depth){
	if (depth > 0){
		assert(prev != nullptr);
		return prev->detech_front(depth-1);
	}
	else if (depth == 0){
		VarMask* temp = prev;
		prev = nullptr;
		return temp;
	}
	else{
		throw std::runtime_error{"depth > list length in VarMask detech_front"};
	}
}

int VarMask::get_maxDepth() const{
	if (prev == nullptr){
		return 0;
	}
	else{
		return prev->get_maxDepth() + 1;
	}
}

MathExprNode* VarMask::transform_leaf(MathExprNode* expr) const{
	throw std::runtime_error("VarMask: transform_leaf() not implemented for " + this->getName());
}

MathExprNode* VarMask::transform(MathExprNode* expr) const {
	// assert(prev == nullptr);
	if (expr->is_leaf()){ 
		if (prev != nullptr){
			return this->transform_leaf(prev->transform_leaf(expr)); 
		}
		else{
			return this->transform_leaf(expr); 
		}
	}
	
	std::vector<MathExprNode*> args;
	int code;
	MathExprNode::NodeType T;
	
	T = expr->decouple(code, args); // This deletes expr
	
	for (int i=0; i<args.size(); i++){
		args[i] = this->transform(args[i]);
	}
	
	MathExprNode* temp;
	switch (T){
		case MathExprNode::NodeType::SumNode:
			return MExprSigma(args);
		case MathExprNode::NodeType::MultNode:
			return MExprMult(args[0], args[1]);
		case MathExprNode::NodeType::DivNode:
			return MExprDiv(args[0], args[1]);
		case MathExprNode::NodeType::AbsNode:
			temp = MExprAbs(args[0]);
			if (code == 1){
				temp -> scale (-1.0);
			}
			return temp;
		default:
			throw std::runtime_error("VarMask: Unknown operator node encountered in transform() function");
	}
}


} //namespace ethelo