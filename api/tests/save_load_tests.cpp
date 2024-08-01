#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include "../api.hpp"
#include "../../engine/ethelo.hpp"
#include "../../engine/mathModelling.hpp"
#include <iostream>

using std::string;
using namespace ethelo;

/*
  This test checks whether a MathProgram can be loaded back unchanged.
*/

class testing_interface: public interface{
	public:
	static MathProgram* preproc_MP(decision& dec){
		return interface::preproc_MP(dec);
	}
};

inline void saveLoadTest(decision& dec){
	const string temp_dec_ID="TEST_DEC";
	const string temp_code_ver = "TEST_CODE";
	const MathProgram* MP0 = testing_interface::preproc_MP(dec);
	
	// save
	std::ostringstream oss;
	MP0->save(oss, temp_dec_ID, temp_code_ver);
	
	// load
	std::istringstream iss(oss.str());
	const MathProgram* MP1 = MathProgram::loadFromStream(iss, dec, temp_dec_ID, temp_code_ver);
	
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
		for (int i=0; i<dispLS0.size(); i++){
			REQUIRE(dispLS0[i]->is_similar(dispLS1[i]));
		}
	}

	delete MP0;
	delete MP1;
}

/*======== TEST CASES ========*/
TEST_CASE("Invalid preproc data", "[SaveLoad]"){
	decision dec (
		{option("op1", {{"a", 1}, {"b", 2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("cons", "[$a] >= 0")},
		{/* no display */},
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	const MathProgram* MP0 = testing_interface::preproc_MP(dec);
	std::ostringstream oss;
	MP0->save(oss, "Hash", "CodeVer");
	
	const string preproc = oss.str();
	
	SECTION("WrongHashValue"){
		std::istringstream iss0(preproc);
		REQUIRE_THROWS_AS(
			MathProgram::loadFromStream(iss0,dec, "WrongHash", "CodeVer"),
			std::invalid_argument);
	}

	SECTION("WrongCodeVer"){
		std::istringstream iss1(preproc);
		REQUIRE_THROWS_AS(
			MathProgram::loadFromStream(iss1,dec, "Hash", "WrongCodeVer"),
			std::invalid_argument);
	}
	
}
/*======== Test for detail sets ========*/
TEST_CASE("Detail Set Tests", "[SaveLoad]"){
	decision dec (
		{option("op1", {{"a", 1}, {"b", 2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("cons1", "[x] >= 0"),
		 constraint("cons2", "[$a] >= 0"),
		 constraint("cons3", "[$b] >= 0"),
		 constraint("cons4", "[10 + $a + $b] >= 0")
		},
		{/* no display */},
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	
	saveLoadTest(dec);
}

/*======== Operator Tests ========*/
TEST_CASE("Addition Test", "[SaveLoad]"){
	decision dec (
		{option("op1", {{"a", 1}, {"b", 2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("cons1", "[$a + $b] >= 0"),
		 constraint("cons2", "[4 + $b] >= 0"),
		 constraint("cons3", "[$a + 6] >= 0"),
		 constraint("cons4", "[10 + $a + $b] >= 0")
		},
		{/* no display */},
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	
	saveLoadTest(dec);
}

TEST_CASE("Subtraction Test", "[SaveLoad]"){
	decision dec (
		{option("op1", {{"a", 1}, {"b", 2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("cons1", "[$a - $b] >= 0"),
		 constraint("cons2", "[4 - $b] >= 0"),
		 constraint("cons3", "[$a - 6] >= 0"),
		 constraint("cons4", "[10 - $a - $b] >= 0"),
		 constraint("cons5", "[10 + $a - $b] >= 0"),
		},
		{/* no display */},
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	
	saveLoadTest(dec);
}
	
TEST_CASE("multiplication Test", "[SaveLoad]") {
	decision dec (
		{option("op1", {{"a", 1}, {"b", 2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("cons1", "[$a * $b] >= 0"),
		 constraint("cons2", "[2 * $b] >= 0"),
		 constraint("cons3", "[$a * 2] >= 0"),
		 constraint("cons4", "[10 * $a * $b] >= 0"),
		 constraint("cons5", "[10 + $a * $b] >= 0"),
		},
		{/* no display */},
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	
	saveLoadTest(dec);
}

TEST_CASE("division Test", "[SaveLoad]") {
	decision dec (
		{option("op1", {{"a", 1}, {"b", 2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("cons1", "[$b / $a] >= 0"),
		 constraint("cons2", "[1 / $a] >= 0"),
		 constraint("cons3", "[$a / 2] >= 0"),
		 constraint("cons4", "[$b / $a + 1] >= 0"),
		 constraint("cons5", "[1 + $b / $a] >= 0"),
		},
		{/* no display */},
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	
	saveLoadTest(dec);
}

TEST_CASE("absolute value Test", "[SaveLoad]") {
	decision dec (
		{option("op1", {{"a", 1}, {"b", -2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("cons1", "[abs($a)] >= 0"),
		 constraint("cons2", "[abs($b)] >= 0"),
		 constraint("cons3", "[0-abs($a)] >= 0"), //see comment #
		 constraint("cons4", "[0-abs($b)] >= 0")
		},
		{/* no display */},
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	// #: There is some weird issue with compiling expressions starting 
	// with '-' that I cannot resolve. It has to do with grammer in 
	// engine/language/expression.g and evaluator in engine/evaluate.cpp
	saveLoadTest(dec);
}

TEST_CASE("Square Root Test", "[SaveLoad]") {
	decision dec (
		{option("op1", {{"a", 1}, {"b", 2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("cons1", "[sqrt($a)] >= 0"),
		 constraint("cons2", "[0-sqrt($a)] >= 0")
		},
		{/* no display */},
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	
	saveLoadTest(dec);
}

TEST_CASE("MultiLayer Test", "[SaveLoad]"){
	decision dec (
		{option("op1", {{"a", 1}, {"b", 2}}),
		option("op2", {{"a", 3}, {"b", 4}})},
		{/* no criteria */},
		{/* no fragment */},
		{constraint("crazy", 
		"[sqrt(abs(($b * $a + 1) / $a)) - abs($a)] >= 0")},
		{}, // no display
		arma::mat({{0.0,0.0}}), // votes
		arma::mat(), // weights
		arma::mat(), // exclusion
		0.0 // CI
	);
	
	saveLoadTest(dec);
}
