#include "../mathModelling.hpp"
#include "../ethelo.hpp"
#include <iomanip>
#include <set>
using namespace std;

namespace ethelo{

const double MathProgram::epsilon = std::numeric_limits<double>::epsilon();
const double MathProgram::INFTY = 2e19;

/*================ MathCons =============*/
MathProgram::MathCons::MathCons():
	lb{0.0}, ub{0.0}, expr{nullptr}, Type{MathProgram::ConsType::VOID}, detail_set_id{-1}, is_relaxable{true}
	{	}

MathProgram::MathCons::MathCons(MathExprNode* expr, double lb, double ub, int detail_set_id, bool is_relaxable):
	lb{lb}, ub{ub}, expr{expr}, Type{MathProgram::Cons_Classify(expr)},
	detail_set_id{detail_set_id},
	is_relaxable{is_relaxable}
	{	}
	
bool MathProgram::MathCons::operator ==(const MathProgram::MathCons& other) const{
	return (Type == other.Type) &&
	       (abs(lb - other.lb) < MathProgram::epsilon) &&
	       (abs(ub - other.ub) < MathProgram::epsilon) &&
		   (is_relaxable == other.is_relaxable) &&
		   (detail_set_id == other.detail_set_id) && 
		   (expr -> is_similar(other.expr));
}
/*=============== MathProgram =====================*/
MathProgram::MathProgram(const problem& p):
	VM{nullptr}, p{p}, excl_added{false}
		{}
		
MathProgram::MathProgram(const FixVar_Mask& VM, const problem& p, bool autofill, bool includeExcl):
	VM{VM.deepcopy()}, p{p}, 
	excl_added{autofill && includeExcl}
	{
	if (autofill){
		fillWithEval(includeExcl);
	}
}
	

void MathProgram::fillWithEval(bool includeExcl){
	evaluator eval(p);
	const auto& cons = p.constraints();
	const auto& epsilon = MathProgram::epsilon;
	
	std::vector<MathExprNode*> ExprList;
	detail_sets.clear();
	
	eval.translate(static_cast<FixVar_Mask*>(this->VM), ExprList,detail_sets);
	
	// ConsList.resize(ExprList.size());
	ConsList.clear();

	for (int i = 0; i < p.constraints().size(); i++) {
		const auto& cons = p.constraints()[i];
		// bool encountered_blacklisted_detail = false;
		
		if (ExprList[i] == nullptr){
			continue;
		}
		ConsList.push_back(
			MathCons(ExprList[i], // expr
				(cons.lbound() ? (cons.lbound().get()) : -MathProgram::INFTY), // lb
				(cons.ubound() ? (cons.ubound().get()) : MathProgram::INFTY), // ub
				i, // detail_set_id
				cons.is_relaxable()
				)  
				);
	}
	//Extract Trees for exclusions
	for (int i = p.constraints().size(); i < p.constraints().size() + p.exclusions().n_rows; i++){
		if (includeExcl){
			ConsList.push_back(	MathCons(ExprList[i], 1.0, MathProgram::INFTY, -1));
		}
		else{
			delete ExprList[i];
			ExprList[i] = nullptr;
		}
	}
	
	// fill out display values
	displayList.clear();
	eval.translate_displays(static_cast<FixVar_Mask*>(this->VM),displayList);
	
}

void MathProgram::print(std::ostream& out) const{
	
	out << "======== Printing MathProgram Tree ... =======";

	for (int i = 0; i < ConsList.size(); i++) {
		out << "\nMP_Cons "<< i+1 << ":";
		out << " LB="<< ConsList[i].lb << ", UB=" << ConsList[i].ub << "{";

		if (ConsList[i].expr != nullptr){
			ConsList[i].expr->print(out);
		}else{
			out << " Relaxed ";
		}

		out << "\n}\n";
	}
}

void MathProgram::apply_mask(VarMask* mask){
	
	assert(mask->is_clean());
	assert(mask->n_var_orig() == this->getVM()->n_var());

	for (auto& cons : ConsList){
		cons.expr = mask->transform(cons.expr);
		cons.Type = Cons_Classify(cons.expr);
		// keep bounds unchanged
	}
	
	mask->addToFront(VM);
	VM = mask;

}

MathProgram::ConsType MathProgram::Cons_Classify(const MathExprNode* Mexpr){
	if (Mexpr == nullptr ){ return VOID;}

	// Linear
	if (Mexpr->is_linear()){ return Linear;}

	// Fractions
	if (Mexpr->is_fraction()){
		double lb,ub;
		static_cast<const DivNode*>(Mexpr)->getArg2()->predict_bound(lb,ub);
		if ((lb >= 0.0) || (ub <= 0.0)){
			return Frac_1;
		}
	}

	//Quadratics
	if (Mexpr->is_quadratic()){
		return Quad;
	}

	//default
	return Others;
}

bool MathProgram::is_linearizable() const{
	for (auto& cons: ConsList){
		switch (cons.Type){
			case MathProgram::ConsType::VOID:
			case MathProgram::ConsType::Linear:
			case MathProgram::ConsType::Frac_1:
			case MathProgram::ConsType::Quad:
				continue;
			default:
				return false;
		}
	}
	return true;
}

bool MathProgram::is_linear() const{
	for (auto& cons : ConsList){
		switch (cons.Type){
			case MathProgram::ConsType::VOID:
			case MathProgram::ConsType::Linear:
				continue;
			default:
				return false;
		}
	}
	return true;
}

void MathProgram::linearize(bool easy){
	if (is_linear()){ return;}
	

	const int n = this->n_var();

	// put constraints into bins
	vector<int> Linear_id, Fraction_id ,Quad_id, Others_id;
	for (int i=0; i<ConsList.size(); i++){
		switch (ConsList[i].Type){
			case MathProgram::ConsType::VOID:	continue;
			case MathProgram::ConsType::Linear:
				Linear_id.push_back(i); 		continue;
			case MathProgram::ConsType::Frac_1:
				Fraction_id.push_back(i); 		continue;
			case MathProgram::ConsType::Quad:
				if (!easy){
					Quad_id.push_back(i);			continue;
				}
			default:
				if (easy){
					Others_id.push_back(i);
					/*
					// Free ignored expressions to avoid memory leak
					delete ConsList[i].expr;
					ConsList[i].expr = nullptr;
					ConsList[i].Type = MathProgram::ConsType::VOID;*/
				}else{
					throw runtime_error("MathProgram: Attempting to linearize a non-linearizable program");
				}
		}
	}

	std::vector<MathCons> OrigConsList;
	std::swap(ConsList, OrigConsList);

	ConsList.clear();
	// First handle Linear constraints
	for (auto i: Linear_id){
		// double lb,ub;
		// const MathCons& cons = OrigConsList[i];
		// static_cast<LinExp*>(cons.expr)->predict_bound(lb,ub); // does this do anything?

		ConsList.push_back(OrigConsList[i]);
	}
	
	// Also copy the other constraints
	for (auto i: Others_id){
		ConsList.push_back(OrigConsList[i]);
	}

	// Now fractional constraints
	for (auto i: Fraction_id){
		MathCons& cons = OrigConsList[i];
		DivNode* node = static_cast<DivNode*>(cons.expr);
		cons.expr = nullptr;
		std::vector<MathExprNode*> argls;
		int code;

		node->decouple(code, argls);

		double denum_lb, denum_ub;
		argls[1]->predict_bound(denum_lb, denum_ub);

		if (denum_ub <0.0){
			//negate constraint
			std::swap(cons.lb, cons.ub);
			cons.lb *= -1.0;
			cons.ub *= -1.0;
			argls[1]->scale(-1.0);
		}

		//Split into linear constraints
		MathExprNode *num, *denum;
		MathCons tempCons;

		if (cons.lb > -MathProgram::INFTY){
			num = argls[0]->deepcopy();
			denum = argls[1]->deepcopy();

			tempCons.lb = 0.0;
			tempCons.ub = MathProgram::INFTY;
			tempCons.expr = MExprSub(num, denum->scale(cons.lb));
			tempCons.Type = MathProgram::ConsType::Linear;

			ConsList.push_back(tempCons);
		}
		if (cons.ub < MathProgram::INFTY){
			num = argls[0]->deepcopy();
			denum = argls[1]->deepcopy();

			tempCons.lb = -MathProgram::INFTY;
			tempCons.ub = 0.0;
			tempCons.expr = MExprSub(num, denum->scale(cons.ub));
			tempCons.Type = MathProgram::ConsType::Linear;
			
			ConsList.push_back(tempCons);
		}

		//clean up to avoid memory leak

		delete argls[0];
		delete argls[1];
		continue;
	}

	// Finally quadratic constraints

	if (Quad_id.size() == 0){
		// no quadratic constraints
		return;
		}

	RLT_Mask *QuadMask = new RLT_Mask(n);
	QuadExprNode* tempQuadExpr;

	// Transform into QuadExprNode, also extract signature
	for (auto i : Quad_id){
		tempQuadExpr =  new QuadExprNode(OrigConsList.at(i).expr);
		OrigConsList.at(i).expr = tempQuadExpr;

		QuadMask->signal_terms(tempQuadExpr);

		ConsList.push_back(OrigConsList.at(i));

	}

	//Transform all other constraints with new mask, also handles linearization
	QuadMask->update();
	apply_mask(QuadMask);

	// Append extra constraints due to RLT Linearization
	LinExp *tempLinExp;
	const std::map<QuadTermID, int>& VarMap = QuadMask->get_map();

	for (const std::pair<QuadTermID, int>& kv_pair : VarMap){
		const int i{kv_pair.first.i}, j{kv_pair.first.j};
		const int y_id{kv_pair.second};

		arma::vec a(QuadMask->n_var(), arma::fill::zeros);

		// y_{i,j} <= x_i
		a[i] = -1.0; a[y_id] = 1.0;
		ConsList.push_back(
			MathCons(new LinExp(this->VM, a, 0.0),
				-MathProgram::INFTY,
				0.0 + MathProgram::epsilon, -1));

		// y_{i,j} <= x_j
		a[i] = 0.0; a[j] = -1.0;
		ConsList.push_back(
			MathCons(new LinExp(this->VM, a, 0.0),
				-MathProgram::INFTY,
				0.0 + MathProgram::epsilon, -1));

		// y_{i,j} >= x_i + x_j - 1
		a[i] = -1.0;
		ConsList.push_back(
			MathCons(new LinExp(this->VM, a, 0.0),
				- 1.0 - MathProgram::epsilon,
				MathProgram::INFTY, -1));
	}
}

void MathProgram::save(ostream& fout, const std::string& decHashed, const std::string& codeVer) const{
	assert(VM->is_identity()); 
	assert(!excl_added);// only intended for preproc_MP
	// ofstream fout(path);
	fout << decHashed << endl;
	fout << "v"+codeVer << endl; // just so that the string is non-empty
	
	fout << setprecision(15);
	fout << VM->n_var() << endl; // print # of vars
	
	// Print detail lists
	const int n_detailSet = detail_sets.size();
	fout << n_detailSet << endl;
	for (int i=0; i<n_detailSet; i++){
		fout << detail_sets.at(i).size() << endl;
		for (const auto& detail: detail_sets.at(i)){
			fout << detail << " ";
		}
		fout << endl;
	}
	
	// save constraints
	fout << ConsList.size() << endl;
	for ( auto& cons: ConsList){
		fout << cons.lb << " " << cons.ub << " " << cons.detail_set_id << " " << cons.is_relaxable << endl;
		cons.expr->save(fout);
	}
	
	// save display list
	fout << displayList.size() << endl;
	for (auto& dispExpr: displayList){
		dispExpr->save(fout);
	}
}

MathProgram* MathProgram::loadFromStream(istream& fin, const problem& p, const string& decHashed, const string& codeVer){
	// ifstream fin(path);
	// Assert versions
	string line;
	fin >> line;
	if (line != decHashed){
		throw invalid_argument("Decision Mismatched for Preproc Data");
	}
	fin >> line;
	if (line != "v"+codeVer){
		throw invalid_argument("Preproc Data for older version detected");
	}
	// Setup mask & MathProgram skeleton
	int n_var;
	fin >> n_var;
	assert(n_var == p.dim());
	FixVar_Mask temp_VM(n_var);
	MathProgram* MP = new MathProgram(temp_VM, p, false, false);
    
	// load details
	int n_detailSet, tempInt;
	string tempStr;
	fin >> n_detailSet;
	MP->detail_sets.resize(n_detailSet);
	for (int i=0; i<n_detailSet; i++){
		fin >> tempInt;
		for (int j=0; j<tempInt; j++){
			fin >> tempStr;
			MP->detail_sets[i].insert(tempStr);
		}
	}
	
	// load Constraints
	int n_cons;
	fin >> n_cons;
	MP->ConsList.resize(n_cons);
	double lb,ub;
	int set_id;
	bool relaxable;
	MathExprNode* Mexpr;
	for (int i=0;i<n_cons; i++){
		fin >> lb >> ub >> set_id >> relaxable;
		Mexpr = loadMExprNode(MP->getVM(), fin);
		MP->ConsList[i] = MathCons(Mexpr, lb, ub, set_id, relaxable);
	}
	
	// load display values
	int n_disp;
	fin >> n_disp;
	MP->displayList.resize(n_disp);
	for (int i=0; i<n_disp; i++){
		MP->displayList[i] = loadMExprNode(MP->getVM(), fin);
	}
	return MP;
}
	
MathProgram* MathProgram::createImage(const FixVar_Mask& VM_new) const{
	assert(!excl_added);
	assert(this->VM->getName()=="FixVar_Mask" && this->VM->is_simple() ); // only to be called on preproc_MP
	// assert(this->VM->n_var_orig() == VM_new.n_var());
	
	MathProgram* tempMP = new MathProgram(*static_cast<FixVar_Mask*>(this->VM), p, false);
	
	tempMP->detail_sets = this->detail_sets;
	
	// filter constraints
	for ( const auto& cons: ConsList ){
		bool skipCons = false;
		if (cons.is_relaxable && cons.detail_set_id != -1){
			// check if blacklisted detail is involved
			for (const auto& used_detail : detail_sets[cons.detail_set_id] ){
				if (p.is_detail_excluded(used_detail)){
					skipCons = true;
					break;
				}
			}
		}

		if (!skipCons){
			tempMP->ConsList.push_back(
				MathCons(cons.expr->deepcopy(),
					cons.lb, cons.ub, cons.detail_set_id,
					cons.is_relaxable));
		}
	}
	
	// apply Mask
	tempMP->apply_mask(VM_new.deepcopy());
	
	// add exclusions
	tempMP->addExcl();
	return tempMP;
}

void MathProgram::addExcl(){
	assert( !excl_added);
	
	const FixVar_Mask* revMask;
	if (VM->n_var() == p.dim()){
		revMask = nullptr;
	}
	else if (VM->n_var_orig(0) == p.dim()){
		assert(VM->getName() == "FixVar_Mask");
		revMask = static_cast<const FixVar_Mask*>(VM);
	}
	else{
		throw runtime_error{"ill-formed VM encountered in MathProgram addExcl"};
	}
	
	excl_added = true;
	// procedure below should mimic behavior in evaluate::translate_expr
	for (int i = 0; i < p.exclusions().n_rows; i++){			
		arma::vec excl = arma::vectorise(arma::mat(p.exclusions().row(i)));
		
		assert(p.dim() == excl.size());
		
		int a_size = (revMask == nullptr?p.dim():revMask->n_var());
		arma::vec a(a_size, arma::fill::zeros);
		// arma::vec a{arma::zeros(a_size)};
		double b = 0;
		int id;
		for (int j=0; j<excl.size(); j++){
			id = (revMask == nullptr? j:revMask->get_mask_id(j));
			// assert(id < a_size);
			if (id == -1){
				b += abs(excl[j] - revMask->get_xVec()[j]);
			}
			else if (excl[j] == 0.0){
				a[id] = 1.0;
			}
			else{
				a[id] = -1.0;
				b += 1;
			}
		}
		ConsList.push_back(MathCons(new LinExp(VM, a, b), 1.0, MathProgram::INFTY, -1));
						
	}
}

MathProgram::~MathProgram(){
	// delete constraints
	for (auto& cons : ConsList){
		delete cons.expr;
		}
		
	// delete displays
	for (auto& Mexpr: displayList){
		delete Mexpr;
	}
	delete VM;
}

void MathProgram::assert_similar(const MathProgram& other) const{	
	assert(VM->n_var() == other.VM->n_var());
	assert(excl_added == other.excl_added );
	assert(ConsList.size() == other.ConsList.size());
	
	for (int i=0;i<ConsList.size(); i++){
		assert(ConsList[i].Type == other.ConsList[i].Type);
		assert(std::abs(ConsList[i].lb - other.ConsList[i].lb) <= MathProgram::epsilon);
		assert(std::abs(ConsList[i].ub - other.ConsList[i].ub) <= MathProgram::epsilon);
		assert(ConsList[i].expr->is_similar(other.ConsList[i].expr));
	}
	
	// display values check
	assert(displayList.size() == other.displayList.size());
	for (int i=0; i<displayList.size(); i++){
		assert(displayList[i]->is_similar(other.displayList[i]));
	}
	
}

void MathProgram::signalBridge(const std::vector<int>& vec){
	assert(FixVar_Mask::checkBridge(vec) == this->n_var());
	bridge = vec;
}

} // namespace ethelo