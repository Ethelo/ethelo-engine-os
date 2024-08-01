#pragma once

#include <cstdint>
#include <memory>
#include <array>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <armadillo>
#include <cppad/cppad.hpp>
#include <plog/Log.h> 

#include "meta.hpp"
#include "util.hpp"
#include "expression.hpp"
#include "detail.hpp"
#include "option.hpp"
#include "fragment.hpp"
#include "constraint.hpp"
#include "display.hpp"
#include "criterion.hpp"
#include "configuration.hpp"
#include "problem.hpp"
// #include "atomic_ethelo.hpp"
#include "evaluate.hpp"
#include "solution.hpp"
#include "solver.hpp"
#include "stats.hpp"
#include "decision.hpp"
#include "exception.hpp"

// #include "solvers/ethelo_tminlp.hpp"
// #include "solvers/solver_bonmin.hpp"
//#include "solvers/solver_cbc.hpp"

#if CPPAD_EIGENVECTOR
#include<Eigen/Core>
#include<cppad/example/cppad_eigen.hpp>
#define CPPAD_TESTVECTOR(Scalar) Eigen::Matrix< Scalar , Eigen::Dynamic, 1>
#endif
