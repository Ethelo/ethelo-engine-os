set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(RapidJSON REQUIRED)

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git describe --always --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")
add_definitions("-DGIT_VERSION=\"${GIT_VERSION}\"")

add_library(ethelo_api STATIC interface.cpp serialization.cpp json_serialization.cpp md5.cpp)
target_include_directories(ethelo_api INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ethelo_api ethelo)

add_library(ethelo_file_solver STATIC file_solver.cpp)
target_include_directories(ethelo_file_solver INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ethelo_file_solver ethelo_api ethelo)

add_executable(complex_decision_tests tests/complex_decision_tests.cpp)
target_link_libraries(complex_decision_tests ethelo_file_solver ethelo_api Catch2::Catch2)

add_executable(save_load_tests tests/save_load_tests.cpp)
target_link_libraries(save_load_tests ethelo_api Catch2::Catch2 ethelo)

add_executable(preproc_tests tests/preproc_tests.cpp)
target_link_libraries(preproc_tests ethelo_file_solver ethelo_api Catch2::Catch2 ethelo)

add_executable(runner runner.cpp)
target_link_libraries(runner ethelo_file_solver)
