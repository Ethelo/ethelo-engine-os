cmake_minimum_required(VERSION 3.0)

project(ethelo)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(3rdparty/Catch2)
add_subdirectory(3rdparty/plog)

add_subdirectory(engine)
add_subdirectory(api)
add_subdirectory(bindings)

enable_testing()
add_test(NAME ExpressionTests COMMAND expression_tests)
add_test(NAME ConstraintTests COMMAND constraint_tests)
add_test(NAME CalculateTests COMMAND calculate_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)
add_test(NAME MPEvalTests COMMAND MP_evaluation_tests)
add_test(NAME SaveLoadTests COMMAND save_load_tests)
add_test(NAME PreprocTests COMMAND preproc_tests)
add_test(NAME ComplexDecisionTests COMMAND complex_decision_tests)
