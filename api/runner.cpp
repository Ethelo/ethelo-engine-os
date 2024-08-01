#include "file_solver.hpp"

int main(int argc, char *argv[]) {
  if(argc != 2) {
    std::cerr << "Please provide the path to the JSON files\n";
    return 1;
  }

  std::string result = ethelo::solve_from_json_dir(std::string(argv[1]));
  std::cout << result;
  return 0;
}
