#define CATCH_CONFIG_MAIN
#include "../ethelo.hpp"
#include "../mathModelling.hpp"
#include <catch2/catch.hpp>

using namespace ethelo;

decision pizza_restaurant_decision(
    {option("pizza_hut"),
     option("dominos_pizza"),
     option("uncle_faiths"),
     option("pizzeria_farina"),
     option("pizza_garden"),
     option("freshslice_pizza"),
     option("panago_pizza"),
     option("megabite_pizza")},
    {/* no criteria */},
    {/* no fragments */},
    {constraint("exactly_one", "[x] = 1"),
	constraint("redundent", "[-1] = -1")},
    {/* no displays */},
    arma::mat({{1, 1, 1, 1, -1, -1, -1, -1},
               {1, 1, 1, 0,  0, -1, -1, -1},
               {1, 1, 0, 0,  0,  0, -1, -1},
               {1, 0, 0, 0,  0,  0,  0, -1}}),
    arma::mat(),
    arma::mat(),
    {0.0 /* CI */});

FixVar_Mask FV(pizza_restaurant_decision.dim());
MathProgram MP(FV,pizza_restaurant_decision, true, true);



inline arma::vec calculate(arma::vec x){
	return solution::compute_fgh(pizza_restaurant_decision, x);
}
/*
// This test does not pass; DO NOT start an expression with '-' character
TEST_CASE("negativeCons", "[calculator]"){
	SECTION("constant eval"){
		REQUIRE(calc.calculate(arma::vec{0, 0, 0, 0, 0, 0, 0, 0})[2] == Approx(-1));
	}
}*/

TEST_CASE("ethelo without fairness in pizza restaurant decision", "[calculator]") {
	pizza_restaurant_decision.linkMathProgram(&MP);
	
    SECTION("scenario with no options") {
        REQUIRE(calculate(arma::vec{0, 0, 0, 0, 0, 0, 0, 0})[0] == Approx(0));
    }

    SECTION("scenario with {pizza_hut}") {
        REQUIRE(calculate(arma::vec{1, 0, 0, 0, 0, 0, 0, 0})[0] == Approx(-1.0/8.0));
    }

    SECTION("scenario with {dominos_pizza}") {
        REQUIRE(calculate(arma::vec{0, 1, 0, 0, 0, 0, 0, 0})[0] == Approx(-(3.0/4.0)/8.0));
    }

    SECTION("scenario with {pizza_hut, dominos_pizza}") {
        REQUIRE(calculate(arma::vec{1, 1, 0, 0, 0, 0, 0, 0})[0] == Approx(-1.0/8.0 - (3.0/4.0)/8.0));
    }

    SECTION("scenario with {pizza_hut, megabite_pizza}") {
        REQUIRE(calculate(arma::vec{1, 0, 0, 0, 0, 0, 0, 1})[0] == Approx(0));
    }
}

TEST_CASE("constraint in pizza restaurant decision", "[calculator]") {
	pizza_restaurant_decision.linkMathProgram(&MP);
	
    SECTION("scenario with no options") {
        REQUIRE(calculate(arma::vec{0, 0, 0, 0, 0, 0, 0, 0})[1] == Approx(0));
    }

    SECTION("scenario with {pizza_hut}") {
        REQUIRE(calculate(arma::vec{1, 0, 0, 0, 0, 0, 0, 0})[1] == Approx(1));
    }

    SECTION("scenario with {dominos_pizza}") {
        REQUIRE(calculate(arma::vec{0, 1, 0, 0, 0, 0, 0, 0})[1] == Approx(1));
    }

    SECTION("scenario with {pizza_hut, dominos_pizza}") {
        REQUIRE(calculate(arma::vec{1, 1, 0, 0, 0, 0, 0, 0})[1] == Approx(2));
    }

    SECTION("scenario with {pizza_hut, megabite_pizza}") {
        REQUIRE(calculate(arma::vec{1, 0, 0, 0, 0, 0, 0, 1})[1] == Approx(2));
    }
}
