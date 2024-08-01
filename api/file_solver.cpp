#include "file_solver.hpp"

std::string ethelo::file2str(std::string path) {
  std::ifstream ifs(path);
  return std::string(
      (std::istreambuf_iterator<char>(ifs)),
      (std::istreambuf_iterator<char>())
  );
}

std::string ethelo::solve_from_json_dir(const std::string& dir) {
  std::string base_path = __FILE__;
  base_path.erase(base_path.find_last_of("/")+1);
  base_path.append("fixtures/");
  
  const std::string decision_json = file2str(dir + "/decision.json");
  const std::string influence_json = file2str(dir + "/influents.json");
  const std::string weights_json = file2str(dir + "/weights.json");
  const std::string config_json = file2str(dir + "/config.json");
  
  const std::string preproc_data = ethelo::interface::preproc(decision_json);

  return ethelo::interface::solve(
    decision_json, influence_json, weights_json, config_json, preproc_data
  );

}

