#define CATCH_CONFIG_MAIN
#include "../ethelo.hpp"
#include "../mathModelling.hpp"
#include <catch2/catch.hpp>

using namespace ethelo;
using std::vector;
using std::string;

TEST_CASE("Addition Test", "[MP]"){
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
	
	FixVar_Mask VM(dec.dim());
	MathProgram MP(VM, dec, true, false);
	dec.linkMathProgram(&MP);
	
	auto fgh = solution::compute_fgh(dec, arma::vec{1,1});
	// $a = 4, $b = 6
	SECTION("Size Check")  { REQUIRE(fgh.n_elem == 5);}
	SECTION("$a + $b")     { REQUIRE(fgh[1] == Approx(10.0));}
	SECTION("4 + $b")      { REQUIRE(fgh[2] == Approx(10.0));}
	SECTION("$a + 6")      { REQUIRE(fgh[3] == Approx(10.0));}
	SECTION("10 + $a + $b"){ REQUIRE(fgh[4] == Approx(20.0));}
}

TEST_CASE("Subtraction Test", "[MP]"){
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
	
	FixVar_Mask VM(dec.dim());
	MathProgram MP(VM, dec, true, false);
	dec.linkMathProgram(&MP);
	
	auto fgh = solution::compute_fgh(dec, arma::vec{1,1});
	// $a = 4, $b = 6
		
	SECTION("Size Check")  {REQUIRE(fgh.n_elem == 6);}
	SECTION("$a - $b")     {REQUIRE(fgh[1] == Approx(-2));}
	SECTION("4 - $b")      {REQUIRE(fgh[2] == Approx(-2));}
	SECTION("$a - 6")      {REQUIRE(fgh[3] == Approx(-2));}
	SECTION("10 - $a - $b"){REQUIRE(fgh[4] == Approx(0)); }
	SECTION("10 + $a - $b"){REQUIRE(fgh[5] == Approx(8)); }
}
	
TEST_CASE("multiplication Test", "[MP]") {
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
	
	FixVar_Mask VM(dec.dim());
	MathProgram MP(VM, dec, true, false);
	dec.linkMathProgram(&MP);
	
	auto fgh = solution::compute_fgh(dec, arma::vec{1,1});
	// $a = 4, $b = 6
	SECTION("Size Check")  {REQUIRE(fgh.n_elem == 6);}      
	SECTION("$a * $b")     {REQUIRE(fgh[1] == Approx(24));  }
	SECTION("2 * $b")      {REQUIRE(fgh[2] == Approx(12));  }
	SECTION("$a * 2")      {REQUIRE(fgh[3] == Approx(8));   }
	SECTION("10 * $a * $b"){REQUIRE(fgh[4] == Approx(240)); }
	SECTION("10 + $a * $b"){REQUIRE(fgh[5] == Approx(34));  }
}

TEST_CASE("division Test", "[MP]") {
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
	
	FixVar_Mask VM(dec.dim());
	MathProgram MP(VM, dec, true, false);
	dec.linkMathProgram(&MP);
	
	auto fgh = solution::compute_fgh(dec, arma::vec{1,1});
	// $a = 4, $b = 6
	SECTION("Size Check")  {REQUIRE(fgh.n_elem == 6);}      
	SECTION("$b / $a")     {REQUIRE(fgh[1] == Approx(1.5));  }
	SECTION("1 / $a")      {REQUIRE(fgh[2] == Approx(0.25));  }
	SECTION("$a / 2")      {REQUIRE(fgh[3] == Approx(2));   }
	SECTION("$b / $a + 1"){REQUIRE(fgh[4] == Approx(2.5)); }
	SECTION("1 + $b / $a"){REQUIRE(fgh[5] == Approx(2.5));  }
}

TEST_CASE("absolute value Test", "[MP]") {
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
	FixVar_Mask VM(dec.dim());
	MathProgram MP(VM, dec, true, false);
	dec.linkMathProgram(&MP);
	
	auto fgh = solution::compute_fgh(dec, arma::vec{1,0});
	// $a = 1, $b = -2
	SECTION("Size Check")  {REQUIRE(fgh.n_elem == 5);}      
	SECTION("abs($a)")     {REQUIRE(fgh[1] == Approx(1));  }
	SECTION("abs($b)")     {REQUIRE(fgh[2] == Approx(2));  }
	SECTION("0-abs($a)")    {REQUIRE(fgh[3] == Approx(-1));   }
	SECTION("0-abs($b)")    {REQUIRE(fgh[4] == Approx(-2)); }
}

TEST_CASE("Square Root Test", "[MP]") {
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
	
	FixVar_Mask VM(dec.dim());
	MathProgram MP(VM, dec, true, false);
	dec.linkMathProgram(&MP);
	
	auto fgh = solution::compute_fgh(dec, arma::vec{1,1});
	// $a = 4, $b = 6
	SECTION("Size Check")   {REQUIRE(fgh.n_elem == 3);}      
	SECTION("sqrt($a)")     {REQUIRE(fgh[1] == Approx(2));  }
	SECTION("0-sqrt($a)")   {REQUIRE(fgh[2] == Approx(-2));  }
}

	