#include "../ethelo.hpp"
#include "../mathModelling.hpp"
#include <stdexcept>

namespace ethelo{
using std::vector;
MathExprNode::MathExprNode(NodeType T, const VarMask* VM): Type{T}, VM{VM} {}

MathExprNode::~MathExprNode(){}

void MathExprNode::print(std::ostream& out) const{
	throw std::invalid_argument("MathExprNode: Printing for "+this->getName()+" has yet been implemented");
}
/*double MathExprNode::evaluate( const arma::vec& x) const{
	throw std::invalid_argument("MathExprNode: Evaluation for "+this->getName()+" has yet been implemented");
}*/

void MathExprNode::save(std::ostream& out) const{
	out << Type << std::endl;
	this->save_content(out);
}

void MathExprNode::predict_bound(double& lb, double& ub) const{
		lb = -MathProgram::INFTY; 
		ub = MathProgram::INFTY; 
		// To be override by subclasses
	}
	
bool MathExprNode::is_leaf() const {
	return false;
}


bool MathExprNode::is_similar(const MathExprNode* other) const{
	throw std::runtime_error{"is_similar Not Implemented for "+this->getName()};
}

MathExprNode* MExprSigma(vector<MathExprNode*>& args){
	assert(args.size() != 0);
	
	LinExp* LinTerm = new LinExp(args.at(0)->VM, 0.0);
	bool LinExpUsed = false;
	
	vector<MathExprNode*> tempArgs1, tempArgs2;
	int code;
	
	for (auto& arg: args){
		if (arg->Type == MathExprNode::NodeType::LinearExp){
			LinExpUsed = true;
			*LinTerm += *static_cast<LinExp*>(arg); 
			delete arg;
			continue;
		}
		else if (arg->Type == MathExprNode::NodeType::SumNode){
			arg->decouple(code, tempArgs2);
			for (auto& arg2: tempArgs2){
				if (arg2->Type == MathExprNode::NodeType::LinearExp){
					LinExpUsed = true;
					*LinTerm += *static_cast<LinExp*>(arg2); 
					delete arg2;
				}
				else{
					tempArgs1.push_back(arg2);
				}
			}
		}
		else{
			tempArgs1.push_back(arg);
		}
	}
	if (tempArgs1.size() == 0){
		// all terms are LinExp
		return LinTerm;
	}
	
	if (LinExpUsed){
		tempArgs1.insert(tempArgs1.begin(), LinTerm);
	}
	
	return new SumNode(std::move(tempArgs1));
	
}


//nullptr treated as 0
MathExprNode* MExprAdd(MathExprNode* arg1, MathExprNode* arg2){
	assert(arg1 != nullptr || arg2 != nullptr);
	
	if (arg1 == nullptr){ return arg2;}
	if (arg2 == nullptr){ return arg1;}
	

	assert(arg1->VM == arg2->VM);
	
	// Sum of Linear Expression
	if (arg1->Type == MathExprNode::NodeType::LinearExp &&
		arg2->Type == MathExprNode::NodeType::LinearExp){
		LinExp* node1 = static_cast<LinExp*>(arg1);
		LinExp* node2 = static_cast<LinExp*>(arg2);
		
		*node1 += *node2;
		
		delete arg2;
		return arg1;
	}
	
	// if one of arg1, arg2 is SumNode
	if (arg1->Type == MathExprNode::NodeType::SumNode || arg2->Type == MathExprNode::NodeType::SumNode){
		if (arg1->Type != MathExprNode::NodeType::SumNode ){
			std::swap(arg1, arg2);
		}
		vector<MathExprNode*> args;
		int code;
		arg1->decouple(code, args);
		args.push_back(arg2);
		// static_cast<SumNode*>(arg1)->appendTerm(arg2);
		// return arg1;
		return MExprSigma(args);
	}
	//default
	return new SumNode(arg1, arg2);
}

//nullptr treated as 0
MathExprNode* MExprSub(MathExprNode* arg1, MathExprNode* arg2){
	assert(arg1 != nullptr || arg2 != nullptr);
	
	if (arg2 == nullptr){ return arg1;}
	
	arg2->scale(-1); 
	return MExprAdd(arg1, arg2);
}

//nullptr treated as 1
MathExprNode* MExprMult(MathExprNode* arg1, MathExprNode* arg2){
	assert(arg1 != nullptr || arg2 != nullptr);
	
	if (arg1 == nullptr){ return arg2;}
	if (arg2 == nullptr){ return arg1;}
	
	assert(arg1->VM == arg2->VM);
	
	
	// Multiply by constant
	if (arg1->Type == MathExprNode::NodeType::LinearExp && 
		is_zeros(static_cast<LinExp*>(arg1)->get_coef())){
		// arg1 is constant
		if (static_cast<LinExp*>(arg1)->get_const() == 0.0){	
			// Multiply by arg1 = 0
			delete arg2;		
			return arg1;
		}
		arg2-> scale(static_cast<LinExp*>(arg1)->get_const());
		delete arg1;
		return arg2;
	}
	
	if (arg2->Type == MathExprNode::NodeType::LinearExp &&
		is_zeros(static_cast<LinExp*>(arg2)->get_coef())){
		//arg2 is constant
		if (static_cast<LinExp*>(arg2)->get_const() == 0.0){	
			// Multiply by 0
			delete arg1;		
			return arg2;
		}
		arg1-> scale(static_cast<LinExp*>(arg2)->get_const());
		delete arg2;
		return arg1;
	}
	
	//default
	return new MultNode(arg1, arg2);
}


MathExprNode* MExprDiv(MathExprNode* arg1, MathExprNode* arg2){
	assert(arg1 != nullptr || arg2 != nullptr);
	
	if (arg2 == nullptr){	return arg1;}
	if (arg1 == nullptr){
		return new DivNode(new LinExp(arg2->VM,1.0), arg2);
	}
	
	assert(arg1->VM == arg2->VM);
	
	if (arg2->Type == MathExprNode::NodeType::LinearExp &&
		is_zeros(static_cast<LinExp*>(arg2)->get_coef())){
		//MExprDivide by constant
		arg1->scale (1.0 / (static_cast<LinExp*>(arg2) ->get_const()));
		delete arg2;
		return arg1;
	}
	
	//default
	return new DivNode(arg1, arg2);
}


MathExprNode* MExprAbs(MathExprNode* arg){
	assert(arg != nullptr);
	
	//arg is a LinExp of which the sign can be predicted
	if (arg->Type == MathExprNode::NodeType::LinearExp){
		const arma::vec& v = static_cast<LinExp*>(arg)->get_coef();
		const double d = static_cast<LinExp*>(arg)->get_const();
		
		// Get range of arg1
		double lo{d},up{d}; 
		
		static_cast<LinExp*>(arg) -> predict_bound(lo,up);
		
		if (lo >= 0){
			// Linear expression only produce positive value
			return arg;
		}
		if (up <= 0){
			// Linear expression only produce negative value
			arg->scale(-1.0);
			return arg;
		}
	}
	//default
	return new AbsNode(arg);
}

MathExprNode* MExprSqrt(MathExprNode* arg){
	assert(arg != nullptr);
	
	//If arg is a LinExp representing a constant
	if (arg->Type == MathExprNode::NodeType::LinearExp &&
		is_zeros(static_cast<LinExp*>(arg)->get_coef())){
		double b = static_cast<LinExp*>(arg)->get_const();
		const VarMask* VM = arg->VM;
		delete arg;
		return new LinExp(VM, std::sqrt(b));
	}
	return new SqrtNode(arg);
}

MathExprNode* loadMExprNode(const VarMask* VM, std::istream& fin){
	int type;
	fin >> type;
	switch (type){
		case MathExprNode::NodeType::SumNode:
			return SumNode::load(VM,fin);
			
		case MathExprNode::NodeType::MultNode:
			return MultNode::load(VM,fin);
			
		case MathExprNode::NodeType::DivNode:
			return DivNode::load(VM,fin);
			
		case MathExprNode::NodeType::AbsNode:
			return AbsNode::load(VM,fin);
			
		case MathExprNode::NodeType::LinearExp:
			return LinExp::load(VM,fin);
			
		case MathExprNode::NodeType::SqrtNode:
			return SqrtNode::load(VM, fin);
			
		case MathExprNode::NodeType::QuadExprNode:
			// QuadExprNode are not supposed to be saved. They only appear
			//   when RLT_Mask is applied to a MathProgram
		default:
			throw std::runtime_error("Undefined NodeType Encountered");
	}
}

} // namespace ethelo
