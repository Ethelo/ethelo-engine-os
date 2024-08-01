#define CATCH_CONFIG_MAIN
#include "../api.hpp"
#include "../file_solver.hpp"
#include <catch2/catch.hpp>
#include <iterator>
#include <fstream>
#include <rapidjson/document.h>

using namespace ethelo;

std::string solve_fixture_decision(std::string fixture_name) {
  std::cout << "Running Test : " << fixture_name << std::endl;
  std::string base_path = __FILE__;
  base_path.erase(base_path.find_last_of("/")+1);
  base_path.append("fixtures/");

  return ethelo::solve_from_json_dir(base_path + fixture_name);
}

std::vector<std::string> get_options_from_result(const std::string& result_json, uint index) {
    std::vector<std::string> result;
    rapidjson::Document d;

    d.Parse(result_json.c_str());
    const rapidjson::Value& option_array = d[index]["options"];
    for (rapidjson::SizeType i = 0; i < option_array.Size(); i++) {
      result.push_back(option_array[i].GetString());
    }
    
    return result;    
}

TEST_CASE("budget test decision", "[integration]") {
  SECTION("with a single option vote") {
    WHEN("the solver runs") {
      std::string result = solve_fixture_decision("budget_decision_partial_vote");

      THEN("the solution prefers scenarios without the null-voted options") {
        std::vector<std::string> options = get_options_from_result(result, 1);
        REQUIRE(options.size() == 2);
      }
    }
  }

  SECTION("with a full vote vote") {
    WHEN("the solver runs") {
      std::string result = solve_fixture_decision("budget_decision_full_vote");

      THEN("the solver finds a solution") {
        std::vector<std::string> options = get_options_from_result(result, 1);
        REQUIRE(options.size() > 0);
      }
    }
  }

  SECTION("with a single option vote with xor constraints") {
    WHEN("the solver runs") {
      std::string result = solve_fixture_decision("budget_decision_partial_vote_with_xors");

      THEN("the solver excludes unvoted options even though they're included in XOR constraints") {
        std::vector<std::string> options = get_options_from_result(result, 1);
        REQUIRE(options.size() == 2);
      }
    }
  }
}

TEST_CASE("tax assessment decision", "[integration]") {
  SECTION("with two 'slider' votes") {
    WHEN("the solver runs") {
      std::string result = solve_fixture_decision("tax_assessment_personal_partial_vote");

      THEN("it picks only the two voted-on options") {
        std::vector<std::string> options = get_options_from_result(result, 1);
        REQUIRE(options.size() == 2);
      }
    }
  }
};

TEST_CASE("carbon budget test decision", "[integration]") {
  std::string result = solve_fixture_decision("carbon_budget");
  std::vector<std::string> options = get_options_from_result(result, 1);
  REQUIRE(options.size() > 0);
}

TEST_CASE("criteria voting decision", "[integration]") {
  WHEN("the solver runs") {
    std::string result = solve_fixture_decision("granting_process");

    THEN("it picks only the two voted-on options") {
      std::vector<std::string> options = get_options_from_result(result, 1);
      REQUIRE(options.size() == 2);
    }
  }
}
