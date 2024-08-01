#pragma once
#include <vector>
namespace ethelo
{
	class MathProgram;
	class FixVar_Mask;
    class solver
    {
		FixVar_Mask formFVMask(const problem& p);
    public:
		MathProgram* formMP(const problem& p);
        solution solve(const problem& p);
    };
}
