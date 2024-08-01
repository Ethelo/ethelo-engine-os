#pragma once
#include "VarMask.hpp"
#include <vector>
#include <set>
#include <iostream>

namespace ethelo{

class MathExprNode;
class problem;
class evaluator;
class FixVar_Mask;

/*
	MathProgram is a structure that is intended to be used for reformulating a problem.
	  All constraints are translated into MathExprNodes
	  
	Invariants:
	  - VM != nullptr, VM->n_orig_var() == p.dim()
	  - All expressions in constraints corrsponds to the VarMask VM at all time
*/

class MathProgram{
  public:
	enum ConsType{VOID, Linear, Frac_1, Quad, Others, UNKNOWN};
	struct MathCons{
		ConsType Type;
		double lb,ub;
		bool is_relaxable;
		MathExprNode* expr;
		int detail_set_id = -1;
		
		MathCons(MathExprNode* expr, double lb, double ub, int detail_set_id, bool is_relaxable = true);
		MathCons();
		
		bool isLinear() const {return Type == ConsType::Linear;}
		//comparison operator for testing.
		bool operator == (const MathCons& other) const;
		
	};
	
  private:
	/* fields */
	const problem& p;
	VarMask* VM;

	bool excl_added;			// whether exclusion constraints has been included
	
	
	std::vector<MathCons> ConsList;      // list of constraints
	std::vector<MathExprNode*> displayList; // list of display values 
	std::vector<std::set<std::string>> detail_sets;
	std::vector<int> bridge;
	
	/* member functions */
	
	// prints() prints structure in human-readable format
	void print(std::ostream& out) const;
	
	void fillWithEval(bool includeExcl);	// fill structure by calling evaluator::translate
	void apply_mask(VarMask* mask);
	
	void addExcl(); // add exclusion constraints
  public:
	static ConsType Cons_Classify(const MathExprNode* Mexpr);
	static const double INFTY; 		//Treat this as +infinity
	static const double epsilon; 	// allowed gap for constraints
	
	MathProgram(const FixVar_Mask& VM, const problem& p, bool autofill = true, bool includeExcl = true);
	MathProgram(const problem& p); // empty C'tor for placeholders. 
	
	~MathProgram(); // this deletes VM and dynamically allocated fields in ConsList
	
	
	const size_t n_var() const 		{	return VM->n_var();}
	const VarMask* getVM() const 	{	return VM;}
	const std::vector<MathCons>& getConsList() const{ return ConsList;} 
	const std::vector<MathExprNode*>& getDisplayList() const{
		return displayList;
	}
	const std::vector<std::set<std::string>>& getDetailSets() const{
		return detail_sets;
	}
	const problem* getProblem() const{ return &p;}
	bool is_linearizable() const;
	bool is_linear() const;
	
	/* linearize(easy) modifies the constraints into linear constraints
		non-linearizable and quadratic constraints are untouched when 
		easy=true, otherwise an exception when encountered
	*/
	void linearize(bool easy); 
	
	
	// void save(std::string path);
	void save(std::ostream& fout, const std::string& decHashed, const std::string& codeVer) const;
	static MathProgram* loadFromStream(std::istream& fin, const problem& p, const std::string& decHashed, const std::string& codeVer); // caller is responsible for freeing returned object
	
	/*
		createImage(allowedSig, VM_new) create a new MathProgram(MP) by:
		1. Filter out constraints that uses details that are blacklisted in p
			at the moment of call
		2. Copy remaining constraints to new MP and call apply_mask(VM_new) in new MP
		3. Add exclusion constraints to new MP by calling addExcl()
		** Note that caller is responsible for freeing returned object
	*/
	MathProgram* createImage(const FixVar_Mask& VM_new) const;
	
	
	void assert_similar(const MathProgram& other) const;
	
	/*get-set functions for bridge*/
	bool hasBridge() const{ return bridge.size() > 0;}
	const std::vector<int>& getBridge() const{ return bridge;}
	void signalBridge(const std::vector<int>& vec); // may throw exception

};



}// namespace ethelo