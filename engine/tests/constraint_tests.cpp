#define CATCH_CONFIG_MAIN
#include "../ethelo.hpp"
#include <catch2/catch.hpp>

TEST_CASE("constraint accepts and compiles valid bounds", "[constraint]") {
    SECTION("left lower bound") {
        ethelo::constraint c("left_lower_bound", "1 <= [a + b + c]");
        REQUIRE(c.valid());
        REQUIRE(bool(c.lbound()) == true);
        REQUIRE(bool(c.ubound()) == false);
        REQUIRE(c.lbound().get() == 1);
    }

    SECTION("left upper bound") {
        ethelo::constraint c("left_upper_bound", "10 >= [a + b + c]");
        REQUIRE(c.valid());
        REQUIRE(bool(c.lbound()) == false);
        REQUIRE(bool(c.ubound()) == true);
        REQUIRE(c.ubound().get() == 10);
    }

    SECTION("right lower bound") {
        ethelo::constraint c("right_lower_bound", "[a + b + c] >= 1");
        REQUIRE(c.valid());
        REQUIRE(bool(c.lbound()) == true);
        REQUIRE(bool(c.ubound()) == false);
        REQUIRE(c.lbound().get() == 1);
    }

    SECTION("right upper bound") {
        ethelo::constraint c("right_upper_bound", "[a + b + c] <= 10");
        REQUIRE(c.valid());
        REQUIRE(bool(c.lbound()) == false);
        REQUIRE(bool(c.ubound()) == true);
        REQUIRE(c.ubound().get() == 10);
    }

    SECTION("bound with negative value") {
        ethelo::constraint c("negative_right_lower_bound", "[a + b + c] >= -50.0");
        REQUIRE(c.valid());
        REQUIRE(bool(c.lbound()) == true);
        REQUIRE(bool(c.ubound()) == false);
        REQUIRE(c.lbound().get() == -50.0);
    }

    SECTION("lower and upper bound") {
        ethelo::constraint c("lower_and_upper_bound", "1 <= [a + b + c] <= 10");
        REQUIRE(c.valid());
        REQUIRE(bool(c.lbound()) == true);
        REQUIRE(bool(c.ubound()) == true);
        REQUIRE(c.lbound().get() == 1);
        REQUIRE(c.ubound().get() == 10);
    }

    SECTION("left equality") {
        ethelo::constraint c("left_equality", "1 = [a + b + c]");
        REQUIRE(c.valid());
        REQUIRE(bool(c.lbound()) == true);
        REQUIRE(bool(c.ubound()) == true);
        REQUIRE(c.lbound().get() == 1);
        REQUIRE(c.ubound().get() == 1);
    }

    SECTION("right equality") {
        ethelo::constraint c("right_equality", "[a + b + c] = 1");
        REQUIRE(c.valid());
        REQUIRE(bool(c.lbound()) == true);
        REQUIRE(bool(c.ubound()) == true);
        REQUIRE(c.lbound().get() == 1);
        REQUIRE(c.ubound().get() == 1);
    }
}

TEST_CASE("constraint rejects duplicate bounds", "[constraint]") {
    SECTION("duplicate equality") {
        REQUIRE_THROWS_AS(
            ethelo::constraint("duplicate_equality", "1 = [a + b + c] = 1"),
            ethelo::semantic_error
        );
    }

    SECTION("duplicate lower bound") {
        REQUIRE_THROWS_AS(
            ethelo::constraint("duplicate_lower_bound", "1 <= [a + b + c] >= 1"),
            ethelo::semantic_error
        );
    }

    SECTION("duplicate upper bound") {
        REQUIRE_THROWS_AS(
            ethelo::constraint("duplicate_lower_bound", "10 >= [a + b + c] <= 10"),
            ethelo::semantic_error
        );
    }
}
