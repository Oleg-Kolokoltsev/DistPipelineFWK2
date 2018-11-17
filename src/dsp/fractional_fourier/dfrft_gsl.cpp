//
// Created by morrigan on 18/01/18.
//

#include <exception>
#include <cmath>
#include <iostream>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_vector_double.h>
#include <cstring>

#include <ctime>
#include <chrono>

#include "dfrft_gsl.h"
#include "dfrft_commons.h"

using namespace std;
using namespace dsp;

/*
 * Compute a 1-st order finite difference approximation of the
 * Hermite-Gaussian functions.
 */
DFrFTGSL::tPtrGSLMatrixReal
DFrFTGSL::first_order_HG(size_t N){
    // the output matrix lifetime is managed by a smart pointer container
    auto mat_deleter = [](gsl_matrix *p) { gsl_matrix_free(p); };
    auto HG = tPtrGSLMatrixReal(gsl_matrix_alloc(N, N), mat_deleter);

    // useful aliases to data
    auto p_HG = HG.get();
    gsl_vector_view vec_view;

    // the most of H elements are zero
    gsl_matrix_set_zero(p_HG);

    // the sub-diagonal elements are all 1.0
    vec_view = gsl_matrix_subdiagonal(p_HG, 1);
    gsl_vector_set_all(&vec_view.vector, 1.0);

    // the super-diagonal elements are all 1.0
    vec_view = gsl_matrix_superdiagonal(p_HG, 1);
    gsl_vector_set_all(&vec_view.vector, 1.0);

    // the main diagonal elements
    vec_view = gsl_matrix_diagonal(p_HG);
    for(size_t i = 0; i < N; i++) {
        double val = 2.0 * cos((double) i * 2.0 * M_PI / (double) N) - 4.0;
        gsl_vector_set(&vec_view.vector, i, val);
    }

    // periodicity
    gsl_matrix_set(p_HG, 0, N-1, 1.0);
    gsl_matrix_set(p_HG, N-1, 0, 1.0);

    return HG;
}

/*
 * The output matrix P decomposes an arbitrary N-dimensional vector V
 * into its even and odd components. This matrix maps the even part
 * of V to the first floor(N/2+1) components and the odd part to the
 * remaining components.
 *
 * Matrix P is unitary and symmetric:
 *  -> P = P^-1 = Transpose[P]
 *  -> P*P = I
 */
DFrFTGSL::tPtrGSLMatrixReal
DFrFTGSL::even_odd_decomp(size_t N){
    if(N < 3) throw runtime_error("DFrFTGSL: N need to be >= 3");

    // the output matrix lifetime is managed by a smart pointer container
    auto mat_deleter = [](gsl_matrix *p) { gsl_matrix_free(p); };
    auto P = tPtrGSLMatrixReal(gsl_matrix_alloc(N, N), mat_deleter);

    // useful aliases to data
    auto p_P = P.get();
    gsl_vector_view vec_view;

    // the most of elements in P are zero
    gsl_matrix_set_zero(p_P);

    // r - is the size of unit ant anti-unit r x r matrices
    bool is_odd = (bool) (N % 2) == 1;
    size_t r = is_odd ? (N - 1)/2 : (N - 2)/2;

    // the main diagonal elements
    vec_view = gsl_matrix_diagonal(p_P);
    gsl_vector_set_all(&vec_view.vector, M_SQRT1_2);
    gsl_vector_set(&vec_view.vector, 0, 1.0);

    vec_view = gsl_vector_subvector(&vec_view.vector, N - r, r);
    gsl_vector_set_all(&vec_view.vector, -M_SQRT1_2);

    // the 1-st sub anti-diagonal
    for(size_t i = 1; i < N; i++)
        gsl_matrix_set(p_P, N-i, i, M_SQRT1_2);

    if(!is_odd) gsl_matrix_set(p_P, r+1, r+1, 1.0);

    return P;
}

/*
 * Here we need to define a basis set for FrFT, the eigenvalues of H matrix
 * obtained by SVD decomposition are thrown away and replaced by k_vec, that
 * is used in the next step in the unitary transform computation - the Fa.
 */
void
DFrFTGSL::create_basis(size_t N){
    if(N <= 3) throw runtime_error("N need to be > 3");

    // fill T with a first order discrete Hermite-Gaussians
    auto T = DFrFTGSL::first_order_HG(N);

    // P is an even/odd decomposition unitary matrix
    auto P = DFrFTGSL::even_odd_decomp(N);

    // useful aliases to data
    gsl_matrix_view mat_vew;
    gsl_vector* vec;
    auto p_Res = T.get();
    auto p_P = P.get();

    // sizes of even and odd matrices for P*T*Transpose[P] congruence
    size_t e = (size_t)floor(N/2.0+1.0);
    size_t o = (size_t)floor((N - 1.0)/2.0);

    // even/odd separation, now T contains two block matrices - even and odd
    dsp::congruence_transform(p_Res, p_P);

    // cleanup numeric errors in zero blocks of T
    mat_vew = gsl_matrix_submatrix(p_Res, 0, e, e, o);
    gsl_matrix_set_zero(&mat_vew.matrix);
    mat_vew = gsl_matrix_submatrix(p_Res, e, 0, o, e);
    gsl_matrix_set_zero(&mat_vew.matrix);

    // process even matrix, the sorted even eigenvectors
    // will be stored in the even part of T
    vec = gsl_vector_alloc(e);
    mat_vew = gsl_matrix_submatrix(p_Res, 0, 0, e, e);
    dsp::eigen(&mat_vew.matrix, vec);
    gsl_eigen_symmv_sort(vec, &mat_vew.matrix, GSL_EIGEN_SORT_VAL_DESC);
    gsl_vector_free(vec);

    // process odd matrix the sorted odd eigenvectors
    // will be stored in the odd part of T
    vec = gsl_vector_alloc(o);
    mat_vew = gsl_matrix_submatrix(p_Res, e, e, o, o);
    dsp::eigen(&mat_vew.matrix, vec);
    gsl_eigen_symmv_sort(vec, &mat_vew.matrix, GSL_EIGEN_SORT_VAL_DESC);
    gsl_vector_free(vec);

    // sort eigenvectors according to their HG index iside of T
    vec = gsl_vector_alloc(N);
    for(int k = 0; k < N; k++)
        gsl_vector_set(vec, k, k < e ? 2*k : 2*(k-e)+1);
    gsl_eigen_symmv_sort(vec, p_Res, GSL_EIGEN_SORT_VAL_ASC);

    // create DFrFTGSL orthonormal basis vectors (real)
    auto E = gsl_matrix_alloc(N, N);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, p_P, p_Res, 0.0, E);
    T.reset(), P.reset();

    // create DFrFTGSL orthonormal basis vectors (real)
    auto mat_deleter_complex = [](gsl_matrix_complex *p) { gsl_matrix_complex_free(p); };
    Ec = tPtrGSLMatrixComplex(gsl_matrix_complex_alloc(N, N), mat_deleter_complex);

    for(size_t i = 0; i < N; i++)
        for(size_t j = 0; j < N; j++){
            auto re = gsl_matrix_get(E, i, j);
            gsl_matrix_complex_set(Ec.get(), i, j, gsl_complex_rect(re, 0.0));
        }

    // store DFrFTGSL index vector (needed to compute the Fs transform)
    // the "vec" is augmented with deleter so it not need to be released
    auto cvec_deleter = [](gsl_vector *p) { gsl_vector_free(p); };
    k_vec = tPtrGSLVectorReal(vec, cvec_deleter);

    gsl_matrix_free(E);
}

// the fastest GSL BLAS solution found
void DFrFTGSL::init_transform(double a){

    if(!Ec || !k_vec) throw runtime_error("init_transform: basis not initialized");
    size_t N = k_vec->size;

    // Fa = E*L*Trans[E], do not resize this matrix if not necessary
    if(!Fa || Fa->size1 != N) {
        auto cmat_deleter = [](gsl_matrix_complex *p) { gsl_matrix_complex_free(p); };
        Fa = tPtrGSLMatrixComplex(gsl_matrix_complex_alloc(N, N), cmat_deleter);
    }

    // LE = L*Trans[E]
    auto LE = gsl_matrix_complex_alloc(N, N);

    // first multiplication with diagonal matrix is done manually, it's fast
    gsl_matrix_complex_memcpy(LE, Ec.get());
    gsl_matrix_complex_transpose(LE);
    for(size_t i = 0; i < N; i++){
        auto fi = -M_PI_2 * gsl_vector_get(k_vec.get(), i) * a;
        auto Lii = gsl_complex_polar(1.0, fi);
        auto row = gsl_matrix_complex_row(LE, i);
        gsl_vector_complex_scale(&row.vector, Lii);
    }

    // this is the slowest routine
    // Fa = Ec * LE + 0*Fa, where LE = L*Ec from the previous step
    gsl_blas_zgemm(CblasNoTrans, CblasNoTrans,
                   gsl_complex_rect(1.0, 0.0), Ec.get(), LE,
                   gsl_complex_rect(0.0, 0.0), Fa.get());

    gsl_matrix_complex_free(LE);
}

// this transform takes about 4ms for N = 1024
void DFrFTGSL::dfrft(std::vector<double>& data){

    size_t N = data.size();
    if(!Fa || Fa->size1 != N)
        throw runtime_error("DFrFTGSL is not initialized or has incompatible size");

    // create complex input
    auto in = gsl_vector_complex_alloc(N);
    gsl_vector_complex_set_zero(in);

    auto view_re_out = gsl_vector_complex_real(in);
    for(int i = 0; i < N; i++)
        gsl_vector_set(&view_re_out.vector, i, data[i]);
    data.clear();

    // apply transform
    auto out = gsl_vector_complex_alloc(N);
    gsl_blas_zgemv(CblasNoTrans,
                   gsl_complex_rect(1.0, 0.0), Fa.get(), in,
                   gsl_complex_rect(0.0, 0.0), out);

    // copy result
    data.resize(2*N);
    memcpy(data.data(), gsl_vector_complex_ptr(out, 0), 2*N*sizeof(double));

    gsl_vector_complex_free(in);
    gsl_vector_complex_free(out);
}
