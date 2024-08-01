#include "ethelo.hpp"
#include "MathModel/FixVar_Mask.hpp"
#include "MathModel/MathProgram.hpp"
#include "atomic_ethelo.hpp"

using CppAD::vector;
namespace ethelo
{
    atomic_ethelo::atomic_ethelo(const char *name) :
        CppAD::atomic_base<double>(name,bool_sparsity_enum),p_(NULL), evalCore{nullptr}
    { }

    atomic_ethelo::atomic_ethelo(const char *name,const problem& p) :
        CppAD::atomic_base<double>(name,bool_sparsity_enum),p_(&p),
		x_dim{p.dim()}, evalCore(&p)
    {
		initialize();
    }
	
	atomic_ethelo::atomic_ethelo(const char* name, const MathProgram* MP):
        CppAD::atomic_base<double>(name,bool_sparsity_enum),
		p_(MP->getProblem()), 
		x_dim{MP->n_var()}, evalCore(MP->getProblem())
    {
		initialize(MP);
    }
	
	void atomic_ethelo::initialize(const MathProgram* MP){
		// extract mask for filling in fixed variables
		expand_mask.resize(x_dim);
		if (MP == nullptr || !MP->hasBridge()){
			assert(x_dim == p_->dim());
			x_init = arma::vec(x_dim, arma::fill::zeros);
			for (int i=0;i<x_dim;i++){
				expand_mask[i]=i;
			}
		}else{
			const auto& bridge = MP->getBridge();
			size_t n = bridge.size();
			assert(n>0);
			x_init = arma::vec(n, arma::fill::zeros);
			int pos = 0;
			for (int i=0;i<n;i++){
				if (bridge[i]==-1){
					x_init[i] = 0.0;
				}else if (bridge[i] == -2){
					x_init[i] = 1.0;
				}else{
					expand_mask[pos] = i; pos ++;
				}
			}
			assert(pos == x_dim);
		}
	}

	double atomic_ethelo::evaluate(const arma::vec& x){
		assert(x.n_elem == x_dim);
		
		arma::vec x_expanded(x_init);
		for (int i=0;i<x_dim; i++){
			x_expanded[expand_mask[i]] = x[i];
		}
		return evalCore.eval(x_expanded, true);
	}

    bool atomic_ethelo::forward(size_t                    p ,
                                size_t                    q ,
                                const vector<bool>&      vx ,
                                vector<bool>&            vy ,
                                const vector<double>&    tx ,
                                vector<double>&          ty )
    {
        double eps = 10.0 * std::numeric_limits<double>::epsilon();

        // Set up some dimensions
        size_t n_order  = q + 1;             // number of Taylor coefficients
        size_t n = p_->dim();      // number of active options
        assert(x_dim == tx.size() / n_order);      // number of variables
        size_t m = ty.size() / n_order;      // range (should be scalar)

        // Extract the relevant influent matrix and configuration
        auto influents = p_->influents();
        auto config = p_->config();

        size_t N = influents.n_rows;   // number of respondents

        // return flag
        bool ok = q <= 1;
        if( ! ok )
            return ok;

        // Check for defining variable information
        if( vx.size() > 0 ){
            vy[0]=false;
            for(int i=0;i<x_dim;i++){
                vy[0] |= vx[i];
            }
        }

        // Extract the point of evaluation, dimension should be same 
		//   as number of active options 
        // Note that the Taylor coefficients are strided uf q>0

		arma::vec x(x_init);
		for (int i=0;i<x_dim;i++){
			x[expand_mask[i]] = tx[i*n_order+0]; 
		}

		// compute ethelo value
		const double ethelo = evalCore.eval(x, true);

        // Now the atomic AD function stuff:

        // Order zero forward mode must always be implemented.
        // y^0 = f( x^0 )
        if( p <= 0 )
            ty[0] = ethelo; // y_0^0
        if( q<=0 )
            return ok;

        // Order one forward mode.
        // This case needed if first order forward mode is used.
        // y^1 = f'( x^0 ) x^1

        // Here we need the dot product of the gradient of Ethelo and x^1

		arma::vec x1(n, arma::fill::zeros);
		for (int i=0;i<x_dim;i++){
			x1[expand_mask[i]] = tx[i*n_order+1];
		}

		// compute gradient, x was seen by evalCore in ethelo value computation
		const arma::vec grad = evalCore.gradient(x, false);

        // More of the atomic AD stuff: compute the dot product of the
        // gradient with x^1
        if( p <= 1 )
          ty[0*n_order+1] = arma::dot(grad,x1); // f'( x^0 ) * x^1
        if( q <= 1 )
            return ok;

        // Make sure we are okay, and have all of the required Taylor coefficients
        assert( !ok );
        return ok;
    }

    bool atomic_ethelo::reverse(size_t                      q  ,
                                const vector<double>&       tx ,
                                const vector<double>&       ty ,
                                vector<double>&             px ,
                                const vector<double>&       py )
    {
        double eps = 10.0 * std::numeric_limits<double>::epsilon();

        // Set up some dimensions
        size_t n_order  = q + 1;
		size_t n = p_->dim();
		assert(x_dim == tx.size() / n_order);
        size_t m = ty.size() / n_order;

        // Extract the relevant influent matrix and configuration
        auto influents = p_->influents();
        auto config = p_->config();

        size_t N = influents.n_rows;   // number of respondents

        // Return flag
        bool ok = q <= 1;

        // Extract the point of evaluation and the first order X(t) sensitivities
		
		arma::vec x(x_init), x1(n, arma::fill::zeros);
		for (int i=0;i<x_dim;i++){
			x[expand_mask[i]] = tx[i*n_order + 0];
			if (q>0){
				x1[expand_mask[i]]= tx[i*n_order + 1];
			}
		}
		
		// compute gradient
		const arma::vec grad = evalCore.gradient(x, true);

        // Now for the atomic AD stuff
        switch(q) {
        case 0:
            // Zero-th order reverse mode
			for (int i=0;i<x_dim;i++){
				px[i] = py[0]*grad[expand_mask[i]];
			}
            assert(ok);
            break;
			
        case 1: 
			// First order reverse mode
			// also compute hessian; x was seen by evalCore in gradient computation
			const arma::mat hess = evalCore.hessian(x, false);
			
			for (int j=0;j<x_dim;j++){
				px[j*n_order+0] = py[0]*grad[expand_mask[j]] + py[1]*arma::dot(hess.row(expand_mask[j]), x1);
				px[j*n_order+1] = py[1]*grad[expand_mask[j]];
			}

            assert(ok);
            break;
        }

        return ok;
    }

    bool atomic_ethelo::rev_sparse_jac(size_t                  p  ,
                                       const vector<bool>&     rt ,
                                       vector<bool>&           st ,
                                       const vector<double>&   x  )
    {   // This function needed if using RevSparseJac or optimize
        // (optimize most of the time)
        size_t n = st.size() / p;
        size_t m = rt.size() / p;
        assert( n == x.size() );
        assert( m == 1 );

        // We are just going to set this to be full, since we may
        // not know any better

        // sparsity for S(x)^T = f'(x)^T * R^T
        for(size_t j = 0; j < p; j++)
            for(size_t i = 0; i < n; i++)
                st[i * p + j] = rt[j];

        return true;
    }
} // end of namespace ethelo
