#include <cppad/cppad.hpp>
#include <map>

namespace ethelo{
	typedef CppAD::AD<double> AD;
	typedef CPPAD_TESTVECTOR(AD) ADvector;
	typedef std::map<std::string, AD> ADmap;
	typedef CPPAD_TESTVECTOR(double) Dvector;
}
