#define CATCH_CONFIG_MAIN
#include "../api.hpp"
#include "../../engine/ethelo.hpp"
#include "../../engine/mathModelling.hpp"
#include "../file_solver.hpp"
#include <catch2/catch.hpp>
#include <iterator>
#include <sstream>
#include <rapidjson/document.h>

using namespace ethelo;


class testing_interface: public interface{
	public:
	static MathProgram* preproc_MP(decision& dec){
		return interface::preproc_MP(dec);
	}
};

// copied from interface.cpp
template<typename Ty>
Ty deserialize(const std::string& format, const std::string& parameter, const std::string& data) {
	try { return serializer<Ty>::create(format)->deserialize(data); }
	catch(const typename serializer<Ty>::parse_error& ex) {
		throw interface::parameter_error(parameter + "_" + format + ": " + ex.what());
	}
}

inline void check_fixture(std::string fixture_name) {
	std::cout << "Running Preproc Test : " << fixture_name << std::endl;
	
	// make file path
	std::string base_path = __FILE__;
	base_path.erase(base_path.find_last_of("/")+1);
	base_path.append("fixtures/");

	std::string dir = base_path + fixture_name;

	// read decision.json
	const std::string decision_json = file2str(dir + "/decision.json");

	decision dec = deserialize<decision>("json", "decision", decision_json);

	// MP when without preproc
	MathProgram* MP0 = testing_interface::preproc_MP(dec);

	// create MP using preproc data
	const std::string preproc_data = testing_interface::preproc(decision_json);
	const std::string dec_hashed = testing_interface::hash(decision_json);
	const std::string ver = testing_interface::version();
	std::istringstream iss(preproc_data);
	MathProgram* MP1 = MathProgram::loadFromStream(iss, dec, dec_hashed, ver);

	// check if MP0,MP1 are the same
	// number of variable
	SECTION("number of variables"){
		REQUIRE(MP0->n_var() == MP1->n_var());
	}
	
	// Detail sets
	SECTION("detail sets"){
		const auto& dtList0 = MP0->getDetailSets();
		const auto& dtList1 = MP1->getDetailSets();
		
		REQUIRE(dtList0.size() == dtList1.size());
		
		for (int i=0;i<dtList0.size(); i++){
			REQUIRE(dtList0[i] == dtList1[i]);
		}
	}
	
	//Constraints
	SECTION("constraint list"){
		const auto& consLS0 = MP0->getConsList();	
		const auto& consLS1 = MP1->getConsList();
		
		REQUIRE(consLS0.size() == consLS1.size());
		REQUIRE(consLS0.size() == dec.constraints().size());
		int n_cons = consLS0.size();
		for (int i=0;i<n_cons;i++){
			REQUIRE(consLS0[i] == consLS1[i]);
		}
	}
	
	//displays
	SECTION("Display Values"){
		const auto& dispLS0 = MP0->getDisplayList();
		const auto& dispLS1 = MP1->getDisplayList();
		
		REQUIRE(dispLS0.size() == dispLS1.size());
		REQUIRE(dispLS0.size() == dec.displays().size());
		for (int i=0; i<dispLS0.size(); i++){
			REQUIRE(dispLS0[i]->is_similar(dispLS1[i]));
		}
	}
	
	// final check
	REQUIRE_NOTHROW(MP0->assert_similar(*MP1));

	delete MP0;
	delete MP1;
}


TEST_CASE("budget test decision", "[integration]") {
  SECTION("with a single option vote") {
    check_fixture("budget_decision_partial_vote");
  }

  SECTION("with a full vote vote") {
    check_fixture("budget_decision_full_vote");
  }

  SECTION("with a single option vote with xor constraints") {
    check_fixture("budget_decision_partial_vote_with_xors");
  }
};


TEST_CASE("tax assessment decision", "[integration]") {
  check_fixture("tax_assessment_personal_partial_vote");
};

TEST_CASE("carbon budget test decision", "[integration]") {
  check_fixture("carbon_budget");
};

TEST_CASE("criteria voting decision", "[integration]") {
    check_fixture("granting_process");
};
