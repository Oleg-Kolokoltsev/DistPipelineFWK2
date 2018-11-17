//
// Created by morrigan on 18/01/18.
//

#ifndef DISTPIPELINEFWK_DFRFT_H
#define DISTPIPELINEFWK_DFRFT_H

#include <memory>
#include <vector>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector_complex_double.h>
#include <gsl/gsl_matrix_complex_double.h>

class DFrFTGSL{
public:

    using tPtrGSLVectorReal = std::shared_ptr<gsl_vector>;
    using tPtrGSLMatrixReal = std::shared_ptr<gsl_matrix>;
    using tPtrGSLMatrixComplex = std::shared_ptr<gsl_matrix_complex>;

    DFrFTGSL(){}

public:
    
    /*
     * create_basis initialize basis functions matrix Ec and an index 
     * vector k_vec, it uses first_order_HG(N) and even_odd_decomp(N) 
     * routines internally.
     * 
     * The basis depends on N only, that is a size of a vector to be 
     * transformed with DFrFTGSL. If a vector size does not change,
     * than a basis need not to be recalculated.
     */
    void create_basis(size_t);
    
    /*
     * DFrFTGSL requires an 'a' parameter to compute the final transform
     * matrix, that is Fa. Reinitialization with 'a' has to be
     * recalculated for each new 'a' parameter. However, if this
     * parameter is still - no recalculation is needed.
     */
    virtual void init_transform(double a);

    /*
     * Direct Discrete Fractional Fourier Transform of real data will return a complex data,
     * so the input vector size will be doubled. Before using this function
     * the transform need to be initialized (see create_basis and init_transform)
     */
    virtual void dfrft(std::vector<double>&);

private:
    static tPtrGSLMatrixReal first_order_HG(size_t N);
    static tPtrGSLMatrixReal even_odd_decomp(size_t N);

protected:
    /*
     * The Discrete Fractional Fourier Transform (DFrFTGSL) is calculated
     * as a matrix product: Fa = E * v^a * Transpose(E), where E is NxN
     * matrix with columns that are orthonormal vectors of length N.
     * These vectors are a discrete analog of real Hermite-Gaussian functions
     * that appear in the continuous FrFT transform. They are ordered in E in
     * a special way where a column with a smaller index correspond
     * to the smallest index of a corresponding Hermite-Gaussian function.
     * When N goes to infinity these discrete functions aspire to their
     * continuous counterparts.
     */

    // orthonormal basis set matrix, depends on N only, it is real but we use a
    // complex data type to simplify further computations
    tPtrGSLMatrixComplex Ec;

    // eigenvalue index vector, depends on N only
    tPtrGSLVectorReal k_vec;

    // current transform matrix: Fa = Ec * v^a * Transpose(Ec)
    tPtrGSLMatrixComplex Fa;
};

#endif //DISTPIPELINEFWK_DFRFT_H
