set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(language)

add_library(ethelo STATIC decision.cpp problem.cpp evaluate.cpp fragment.cpp constraint.cpp display.cpp expression.cpp solution.cpp solvers/solver_bonmin.cpp atomic_ethelo.cpp nuclear_ethelo.cpp solver.cpp solvers/solver_cbc.cpp  solvers/tminlp_Base.cpp solvers/tminlp_MP.cpp solvers/tminlp_LinMP.cpp MathModel/MathExprNode.cpp MathModel/SqrtNode.cpp MathModel/MultNode.cpp MathModel/DivNode.cpp MathModel/AbsNode.cpp MathModel/SumNode.cpp MathModel/LinExp.cpp MathModel/QuadExprNode.cpp MathModel/VarMask.cpp MathModel/FixVar_Mask.cpp MathModel/RLT_Mask.cpp MathModel/MathProgram.cpp)

target_include_directories(ethelo INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ethelo language armadillo bonmin CoinUtils Cbc OsiClp Clp plog)

add_executable(expression_tests tests/expression_tests.cpp)
target_link_libraries(expression_tests ethelo Catch2::Catch2)

add_executable(constraint_tests tests/constraint_tests.cpp)
target_link_libraries(constraint_tests ethelo Catch2::Catch2)

add_executable(calculate_tests tests/calculate_tests.cpp)
target_link_libraries(calculate_tests ethelo Catch2::Catch2)

add_executable(integration_tests tests/integration_tests.cpp)
target_link_libraries(integration_tests ethelo Catch2::Catch2)

add_executable(MP_evaluation_tests tests/calculate_test_MP.cpp)
target_link_libraries(MP_evaluation_tests ethelo Catch2::Catch2)
