#pragma once

namespace ethelo
{
	class MathProgram;
    class solver_bonmin
    {
    public:
		solution s;
		
        void solve(const MathProgram* MP);
    };
}
