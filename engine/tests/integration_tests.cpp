#define CATCH_CONFIG_MAIN
#include "../ethelo.hpp"
#include "../mathModelling.hpp"
#include <catch2/catch.hpp>

using namespace ethelo;

TEST_CASE("pizza restaurant decision", "[integration]") {
    SECTION("without fairness using BONMIN") {
        decision pizza_restaurant_decision(
            {option("pizza_hut"),
             option("dominos_pizza"),
             option("uncle_faiths"),
             option("pizzeria_farina"),
             option("pizza_garden"),
             option("freshslice_pizza"),
             option("panago_pizza"),
             option("megabite_pizza")},
            {}, //no criteria
            {}, //no fragments
            {constraint("exactly_one", "[x] = 1")},
            {}, //no displays
            arma::mat({{1, 0, 0, 1, 0, 1, 0, 0.5},
                       {1, 0, 0, 0, 0, 1, 0, 1},
                       {0, 1, 1, 0, 0, 1, 0, 1},
                       {1, 0, 0, 1, 0, 1, 0, 1}}),
            arma::mat(),
            arma::mat(), // no exclusion
                                    0.0); //CI
									
		FixVar_Mask FV(pizza_restaurant_decision.dim());
		MathProgram MP(FV, pizza_restaurant_decision, true,false);
		pizza_restaurant_decision.linkMathProgram(&MP);

        auto solution = pizza_restaurant_decision.solve();
        REQUIRE(solution.status == "success");
        REQUIRE(arma::norm(solution.x - arma::vec{0, 0, 0, 0, 0, 1, 0, 0}) == Approx(0));
        REQUIRE(solution.options == std::set<std::string>{"freshslice_pizza"});
    }
}

TEST_CASE("pizza decision", "[integration]") {
    SECTION("without fairness using BONMIN") {
        decision pizza_decision(
            {option("pepperoni_mushroom", {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14}}),
             option("large_cheese",       {{"cost", 12},
                                           {"feeds", 6},
                                           {"vegetarian", 1},
                                           {"inches", 20}}),
             option("regular_cheese",     {{"cost", 12},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14}}),
             option("meat_lovers",        {{"cost", 22},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14}}),
             option("veggie_lovers",      {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14}})},
            {}, // no criteria
            {fragment("vegetarian", "$vegetarian[i]"),
             fragment("meat", "1 - $vegetarian[i]")},
            {constraint("veg_min", "[sum[i in x]{@vegetarian}] >= 1"),
             constraint("meat_min", "[sum[i in x]{@meat}] >= 1"),
             constraint("budget", "[$cost] <= 65"),
             constraint("feeds_min", "[$feeds] >= 10")},
            {}, // no displays
            arma::mat({{1, 0, 0, 1, 1},
                       {1, 0, 0, 0, 1},
                       {0, 1, 1, 0, 1},
                       {1, 0, 0, 1, 1}}),
            arma::mat(),
            arma::mat(),// no exclusion
            0.0); //CI

		FixVar_Mask FV(pizza_decision.dim());
		MathProgram MP(FV, pizza_decision, true,false);
		pizza_decision.linkMathProgram(&MP);
		
        auto solution = pizza_decision.solve();
        REQUIRE(solution.status == "success");
    }

    GIVEN("influents have NULL option values") {
        decision pizza_decision(
            {option("pepperoni_mushroom", {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14}}),
             option("large_cheese",       {{"cost", 12},
                                           {"feeds", 6},
                                           {"vegetarian", 1},
                                           {"inches", 20}}),
             option("regular_cheese",     {{"cost", 12},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14}}),
             option("meat_lovers",        {{"cost", 22},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14}}),
             option("veggie_lovers",      {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14}})},
            {}, // no criteria
            {fragment("vegetarian", "$vegetarian[i]"),
             fragment("meat", "1 - $vegetarian[i]")},
            {constraint("veg_min", "[sum[i in x]{@vegetarian}] >= 1"),
             constraint("meat_min", "[sum[i in x]{@meat}] >= 1"),
             constraint("budget", "[$cost] <= 65"),
             constraint("feeds_min", "[$feeds] >= 10")},
            {/* no displays */},
            arma::mat({{1.0, 1.0, 2.0, 2.0, 2.0}, // 2.0  == null vote
                       {1.0, 1.0, 2.0, 2.0, 2.0}}),
            arma::mat(),
            arma::mat(),// no exclusion
            0.0 /* CI */);
			
		FixVar_Mask FV(pizza_decision.dim());
		MathProgram MP(FV, pizza_decision, true,false);
		pizza_decision.linkMathProgram(&MP);

        WHEN("the solver runs") {
            auto solution = pizza_decision.solve();
            REQUIRE(solution.status == "success");

            THEN("the solution prefers scenarios without the null-voted options") {
              REQUIRE(solution.options.size() == 2);
            }
        }
    }

    GIVEN("influents have null on options required by constraints") {
        decision pizza_decision(
            {option("pepperoni_mushroom", {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14}}),
             option("large_cheese",       {{"cost", 12},
                                           {"feeds", 6},
                                           {"vegetarian", 1},
                                           {"inches", 20}}),
             option("regular_cheese",     {{"cost", 12},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14}}),
             option("meat_lovers",        {{"cost", 22},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14}}),
             option("veggie_lovers",      {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14}})},
            {}, // no criteria
            {fragment("vegetarian", "$vegetarian[i]"),
             fragment("meat", "1 - $vegetarian[i]")},
            {constraint("veg_min", "[sum[i in x]{@vegetarian}] >= 1"),
             constraint("meat_min", "[sum[i in x]{@meat}] >= 1"),
             constraint("budget", "[$cost] <= 65"),
             constraint("feeds_min", "[$feeds] >= 10")},
            {/* no displays */},
            arma::mat({{2.0, 1, 1, 2.0, 2.0}, // 2.0  == null vote
                       {2.0, 1, 1, 2.0, 2.0}}),
            arma::mat(),
            arma::mat(),// no exclusion
            0.0 /* CI */);
			
		FixVar_Mask FV(pizza_decision.dim());
		MathProgram MP(FV, pizza_decision, true,false);
		pizza_decision.linkMathProgram(&MP);

        WHEN("the solver runs") {

            auto solution = pizza_decision.solve();
            REQUIRE(solution.status == "success");

            THEN("the solution still includes null voted options in order to satisfy the constraints") {
              REQUIRE(solution.options.size() == 3);
            }
        }
    }
}

TEST_CASE("pizza has no mushroom decision", "[integration]") {
    SECTION("single influent has null option required by constraints") {
        decision pizza_decision(
            {option("pepperoni_mushroom", {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14},
                                           {"no_mushroom",0}}),
             option("large_cheese",       {{"cost", 12},
                                           {"feeds", 6},
                                           {"vegetarian", 1},
                                           {"inches", 20},
                                           {"no_mushroom",1}}),
             option("regular_cheese",     {{"cost", 12},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14},
                                           {"no_mushroom",1}}),
             option("meat_lovers",        {{"cost", 22},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14},
                                           {"no_mushroom",1}}),
             option("veggie_lovers",      {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14},
                                           {"no_mushroom",0}})},
            {}, // no criteria
            {fragment("vegetarian", "$vegetarian[i]"),
             fragment("meat", "1 - $vegetarian[i]"),
             fragment("contains_mushrooms","$mushroom[i]")},
            {constraint("budget", "[$cost] <= 65"),
             constraint("feeds_min", "[$feeds] >= 10"),
             constraint("has_no_mushroom","[$no_mushroom] >= 1 ")
             },
            {/* no displays */},
            arma::mat({{0.3, 1, 1, 2.0, 2.0}}), // 2.0  == null vote
            arma::mat(),
            arma::mat(),// no exclusion
            0.0 /* CI */);
			
		FixVar_Mask FV(pizza_decision.dim());
		MathProgram MP(FV, pizza_decision, true,false);
		pizza_decision.linkMathProgram(&MP);
		
        auto solution = pizza_decision.solve();
        REQUIRE(solution.status == "success");
        INFO(" Solution is: "<<solution.x.t());
        REQUIRE(arma::all(solution.x == arma::vec{1,1,1,0,0}));

        THEN("the solution still includes null voted options in order to satisfy the constraints") {
            REQUIRE(solution.options.size() == 3);
        }
    }
}

TEST_CASE("pizza no mushroom decision", "[integration]") {
    SECTION("single influent has null option required by constraints but with positive sum in excluded options") {
        decision pizza_decision(
            {option("pepperoni_mushroom", {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14},
                                           {"mushroom",1}}),
             option("large_cheese",       {{"cost", 12},
                                           {"feeds", 6},
                                           {"vegetarian", 1},
                                           {"inches", 20},
                                           {"mushroom",0}}),
             option("regular_cheese",     {{"cost", 12},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14},
                                           {"mushroom",0}}),
             option("meat_lovers",        {{"cost", 22},
                                           {"feeds", 4},
                                           {"vegetarian", 0},
                                           {"inches", 14},
                                           {"mushroom",0}}),
             option("veggie_lovers",      {{"cost", 18},
                                           {"feeds", 4},
                                           {"vegetarian", 1},
                                           {"inches", 14},
                                           {"mushroom",1}})},
            {}, // no criteria
            {fragment("vegetarian", "$vegetarian[i]"),
             fragment("meat", "1 - $vegetarian[i]"),
             fragment("contains_mushrooms","$mushroom[i]")},
            {constraint("budget", "[$cost] <= 65"),
             constraint("feeds_min", "[$feeds] >= 10"),
             constraint("no_mushroom","[$mushroom] = 0 ")
             },
            {/* no displays */},
            arma::mat({{2.0, 1, 1, 2.0, 2.0}}), // 2.0  == null vote
            arma::mat(),
            arma::mat(),
            0.0 /* CI */);
		
		FixVar_Mask FV(pizza_decision.dim());
		MathProgram MP(FV, pizza_decision, true,false);
		pizza_decision.linkMathProgram(&MP);
		
        auto solution = pizza_decision.solve();
        REQUIRE(solution.status == "success");
        INFO(" Solution is: "<<solution.x.t());
        REQUIRE(arma::all(solution.x == arma::vec{0,1,1,0,0}));

        THEN("the solution still includes null voted options in order to satisfy the constraints") {
            REQUIRE(solution.options.size() == 2);
        }
    }
}
    
