//
// Created by morrigan on 19/01/18.
//

#include <iostream>
#include <iomanip>
#include <complex>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_eigen.h>

#include "dfrft_commons.h"

using namespace std;

void dsp::print_vec(const gsl_vector* v){
    ios::fmtflags f( cout.flags() );
    for(int i = 0; i < v->size; i++)
        cout << right
             << setfill(' ')
             << setprecision(1)
             << setw(12)
             << scientific
             << gsl_vector_get(v, i)
             << " ";
    cout << endl << endl;
    cout.flags( f );
    cout << std::fixed;
}

void dsp::print_mat_r(const gsl_matrix * m){
    ios::fmtflags f( cout.flags() );
    for(int i = 0; i < m->size1; i++) {
        for (int j = 0; j < m->size2; j++) {
            cout << right
                 << setfill(' ')
                 << setprecision(1)
                 << setw(12)
                 << scientific
                 << gsl_matrix_get(m, i, j)
                 << " ";
        }
        cout << endl;
    }
    cout << endl << endl;
    cout.flags( f );
    cout << std::fixed;
}

void dsp::print_mat_c(const gsl_matrix_complex* m){
    ios::fmtflags f( cout.flags() );
    for(int i = 0; i < m->size1; i++) {
        for (int j = 0; j < m->size2; j++) {
            auto c = gsl_matrix_complex_get(m, i, j);
            cout << right
                 << setfill(' ')
                 << setprecision(1)
                 << setw(12)
                 << scientific
                 << "(" << GSL_REAL(c) << ", " << GSL_IMAG(c) << ")"
                 << " ";
        }
        cout << endl;
    }
    cout << endl << endl;
    cout.flags( f );
    cout << std::fixed;
}

void dsp::print_zero_crossings_vec(const gsl_vector* v){
    size_t z = 0;
    size_t N = v->size;

    for(size_t i = 0; i < N - 1; i++)
        z = gsl_vector_get(v,i)*gsl_vector_get(v,i + 1) < 0 ? z+1 : z;

    if(N % 2)
        z = gsl_vector_get(v,N-1)*gsl_vector_get(v, 0) < 0 ? z+1 : z;

    cout << z;
}

void dsp::print_zero_crossings_mat(const gsl_matrix* m){
    for(size_t i = 0; i < m->size2; i++){
        auto col = gsl_matrix_const_column(m, i);
        dsp::print_zero_crossings_vec(&col.vector);
        cout << " ";
    }
    cout << endl << endl;
}

void dsp::congruence_transform(gsl_matrix* A, const gsl_matrix* B){
    size_t N = A->size1;
    if(A->size2 != N || B->size1 != N || B->size2 != N)
        throw runtime_error("congruence_transform: incorrect dimensions");

    auto tmp = gsl_matrix_alloc(N, N);
    gsl_blas_dgemm(CblasNoTrans, CblasTrans, 1.0, A, B, 0.0, tmp);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, B, tmp, 0.0, A);
    gsl_matrix_free(tmp);
}

void dsp::eigen(gsl_matrix* A, gsl_vector* v){
    size_t N = A->size1;
    if(A->size2 != N || v->size != N)
        throw runtime_error("eigen: incorrect dimensions");
    auto tmp = gsl_matrix_alloc(N, N);
    auto eigen_ws = gsl_eigen_symmv_alloc(N);

    gsl_eigen_symmv(A, v, tmp, eigen_ws);
    gsl_matrix_memcpy(A, tmp);

    gsl_matrix_free(tmp);
    gsl_eigen_symmv_free(eigen_ws);
}

