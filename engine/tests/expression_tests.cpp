#define CATCH_CONFIG_MAIN
#include "../ethelo.hpp"
#include <catch2/catch.hpp>

TEST_CASE("expression accepts basic arithmetic", "[expression]") {
	SECTION("constants") {
		REQUIRE(ethelo::expression("positive", "1").valid());
		REQUIRE(ethelo::expression("negative", "-1").valid());
	}
    SECTION("addition") {
        REQUIRE(ethelo::expression("add", "1 + 1").valid());
    }

    SECTION("multiplication") {
        REQUIRE(ethelo::expression("mul", "2 * 2").valid());
    }

    SECTION("division") {
        REQUIRE(ethelo::expression("div", "4 / 2").valid());
    }

    SECTION("addition, multiplication and division") {
        REQUIRE(ethelo::expression("add_mul_div", "4 / (1 + 1) * 2").valid());
    }

    SECTION("unary operations") {
        REQUIRE(ethelo::expression("unary_add", "+1 + -1").valid());
    }

    SECTION("addition with missing operand") {
        REQUIRE_THROWS_AS(
            ethelo::expression("invalid_add", "1 + 1 +"),
            ethelo::syntax_error
        );
    }

    SECTION("absolute value function") {
        REQUIRE(ethelo::expression("abs","abs(-4+2)").valid());
    }

    SECTION("sqrt function") {
        REQUIRE(ethelo::expression("sqrt","sqrt(4)").valid());
    }

}

TEST_CASE("expression rejects erroneous arithmetic", "[expression]") {
    SECTION("multiplication with missing operand") {
        REQUIRE_THROWS_AS(
            ethelo::expression("invalid_mul", "* 2 * 4"),
            ethelo::syntax_error
        );
    }

    SECTION("division with missing operand") {
        REQUIRE_THROWS_AS(
            ethelo::expression("invalid_div", "1 /"),
            ethelo::syntax_error
        );
    }

    SECTION("unmatched parenthesis") {
        REQUIRE_THROWS_AS(
            ethelo::expression("unmatched_paren", "2 * (1 + 1"),
            ethelo::syntax_error
        );
    }
}

TEST_CASE("expression accepts expressions with variables", "[expression]") {
    SECTION("variables in simple expression") {
        REQUIRE(ethelo::expression("var_simple", "(a + b) * c").valid());
    }

    SECTION("variables in complex expression") {
        REQUIRE(ethelo::expression("var_complex", "z * z * ((a + b + 10) * 100 - c) / alpha + beta").valid());
    }

    SECTION("array variables") {
        REQUIRE(ethelo::expression("array_var", "(alpha[0] + beta[5]) * gamma + zeta['key']").valid());
    }
}

TEST_CASE("expression rejects invalid variables", "[expression]") {
    SECTION("variable starting with a number") {
        REQUIRE_THROWS_AS(
            ethelo::expression("bad_var_num", "1a + b"),
            ethelo::syntax_error
        );
    }

    SECTION("variable with symbol") {
        REQUIRE_THROWS_AS(
            ethelo::expression("bad_var_sym", "a@b + c"),
            ethelo::syntax_error
        );
    }
}

TEST_CASE("expression accepts expressions with details", "[expression]") {
    SECTION("details and variables in simple expression") {
        REQUIRE(ethelo::expression("detail_simple", "($a + $b) * c").valid());
    }

    SECTION("details and variables in complex expression") {
        REQUIRE(ethelo::expression("detail_complex", "z * $z * ((a + $b + 10) * 100 - c) / alpha + $beta").valid());
    }

    SECTION("array variables and details") {
        REQUIRE(ethelo::expression("array_detail", "(alpha[0] + $beta[5]) * gamma + $zeta['key']").valid());
    }
}

TEST_CASE("expression accepts aggregate functions", "[expression]") {
    SECTION("aggregate over selected options") {
        REQUIRE(ethelo::expression("sum_selected", "sum[i in x]{$alpha[i] + x[i] * y[i]}").valid());
    }

    SECTION("aggregate over detail group") {
        REQUIRE(ethelo::expression("sum_detail", "sum[i in $group]{$alpha[i] + x[i] * y[i]}").valid());
    }
}

TEST_CASE("expression accepts expressions with fragments", "[expression]") {
    SECTION("fragments and variables in simple expression") {
        REQUIRE(ethelo::expression("fragment_simple", "(@a + @b) * c").valid());
    }

    SECTION("fragments and variables in complex expression") {
        REQUIRE(ethelo::expression("fragment_complex", "z * @z * ((a + @b + 10) * 100 - c) / alpha + @beta").valid());
    }
}

TEST_CASE("expression accepts bounded expressions", "[expression]") {
    SECTION("left lower bound") {
        REQUIRE(ethelo::expression("left_lower_bound", "1 <= [a + b + c]").valid());
    }

    SECTION("left upper bound") {
        REQUIRE(ethelo::expression("left_upper_bound", "10 >= [a + b + c]").valid());
    }

    SECTION("right lower bound") {
        REQUIRE(ethelo::expression("right_lower_bound", "[a + b + c] >= 1").valid());
    }

    SECTION("right upper bound") {
        REQUIRE(ethelo::expression("right_upper_bound", "[a + b + c] <= 10").valid());
    }

    SECTION("lower and upper bound") {
        REQUIRE(ethelo::expression("lower_upper_bound", "1 <= [a + b + c] <= 10").valid());
    }

    SECTION("left equality") {
        REQUIRE(ethelo::expression("lower_upper_bound", "1 = [a + b + c]").valid());
    }

    SECTION("right equality") {
        REQUIRE(ethelo::expression("lower_upper_bound", "[a + b + c] = 1").valid());
    }
}

TEST_CASE("expression rejects invalid bounds", "[expression]") {
    SECTION("bound with variable") {
        REQUIRE_THROWS_AS(
            ethelo::expression("bound_with_var", "d <= [a + b + c]"),
            ethelo::syntax_error
        );
    }

    SECTION("incomplete bound") {
        REQUIRE_THROWS_AS(
            ethelo::expression("incomplete_bound", "<= [a + b + c]"),
            ethelo::syntax_error
        );
    }

    SECTION("strict inequality") {
        REQUIRE_THROWS_AS(
            ethelo::expression("strict_inequality", "1 < [a + b + c]"),
            ethelo::syntax_error
        );
    }
}
