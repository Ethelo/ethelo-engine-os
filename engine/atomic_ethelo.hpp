#pragma once
#include "nuclear_ethelo.hpp"
namespace ethelo
{
    //using CppAD::vector;
	class MathProgram;
    class atomic_ethelo : public CppAD::atomic_base<double>
    {
        const problem* p_;
		std::vector<int> expand_mask;
		const int x_dim = -1;
		arma::vec x_init;
		nuclear_ethelo evalCore;
		
		void initialize(const MathProgram* MP=nullptr);
		
    public:
        // constructor
        atomic_ethelo(const char *name);
        // constructor
        atomic_ethelo(const char *name,const problem& p);
		// constructor
        atomic_ethelo(const char *name,const MathProgram* MP);
		
		double evaluate(const arma::vec& x);
    private:
        // arma::vec mu;
        // arma::mat Q;

        virtual bool forward(size_t                          p ,
                             size_t                          q ,
                             const CppAD::vector<bool>&      vx ,
                             CppAD::vector<bool>&            vy ,
                             const CppAD::vector<double>&    tx ,
                             CppAD::vector<double>&          ty );

        virtual bool reverse(size_t                             q  ,
                             const CppAD::vector<double>&       tx ,
                             const CppAD::vector<double>&       ty ,
                             CppAD::vector<double>&             px ,
                             const CppAD::vector<double>&       py );
        // Apparently this is needed to pass the default build.
        virtual bool rev_sparse_jac(size_t                         p  ,
                                    const CppAD::vector<bool>&     rt ,
                                    CppAD::vector<bool>&           st ,
                                    const CppAD::vector<double>&   x  );
    };

} //End of namespace ethelo
