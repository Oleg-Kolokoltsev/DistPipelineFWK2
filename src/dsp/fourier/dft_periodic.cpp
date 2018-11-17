//
// Created by morrigan on 17/01/18.
//
#include <exception>
#include <stdexcept>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_complex.h>

#include "dft_periodic.h"

using namespace std;

void
DFT::initialize(size_t N, CONTEXT_TYPE ctx_type){

    if(N == 0) throw runtime_error("DFT initialize: N cannot be 0");

    if(ctx_type == REAL || ctx_type == ALL) {

        auto rwt_deleter = [](gsl_fft_real_wavetable *p) { gsl_fft_real_wavetable_free(p); };
        real_wavetable.reset(gsl_fft_real_wavetable_alloc(N), rwt_deleter);

        auto rws_deleter = [](gsl_fft_real_workspace *p) { gsl_fft_real_workspace_free(p); };
        real_workspace.reset(gsl_fft_real_workspace_alloc(N), rws_deleter);
    }

    if(ctx_type == COMPLEX || ctx_type == ALL){
        auto cwt_deleter = [](gsl_fft_complex_wavetable* p){gsl_fft_complex_wavetable_free(p); };
        complex_wavetable = tPtrComplexWavetable(gsl_fft_complex_wavetable_alloc(N), cwt_deleter);

        auto cws_deleter = [](gsl_fft_complex_workspace* p){gsl_fft_complex_workspace_free(p);};
        complex_workspace = tPtrComplexWorkspace(gsl_fft_complex_workspace_alloc(N), cws_deleter);
    }
}

bool
DFT::is_initialized(size_t N, CONTEXT_TYPE ctx_type){
    if(ctx_type == REAL) {
        return (real_wavetable && real_workspace && real_workspace->n == N);
    }else if(ctx_type == COMPLEX){
        return (complex_wavetable && complex_workspace && complex_workspace->n == N);
    }else if(ctx_type == ALL){
        return is_initialized(N, REAL) && is_initialized(N, COMPLEX);
    }
}

void
DFT::dft(vector<double>& s, CONTEXT_TYPE in_ctx_type){
    if(in_ctx_type == ALL)
        throw runtime_error("DFT transform input data type may be only real or comlpex");

    auto N = in_ctx_type == REAL ? s.size() : s.size()/2;
    if(!is_initialized(N, in_ctx_type)) throw runtime_error("DFT: REAL context not initialized");

    if(in_ctx_type == REAL) {
        std::vector<double> out(2 * N);
        gsl_fft_real_transform(s.data(), 1, N, real_wavetable.get(), real_workspace.get());
        gsl_fft_halfcomplex_unpack(s.data(), out.data(), 1, N);
        s = move(out);
    }else if(in_ctx_type == COMPLEX){
        gsl_fft_complex_forward(s.data(), 1, N, complex_wavetable.get(), complex_workspace.get());
    }
}


void
DFT::ift(std::vector<double>& S, CONTEXT_TYPE out_ctx_type){
    auto N = S.size()/2;
    if(!is_initialized(N, COMPLEX)) throw runtime_error("FFT: COMPLEX context not initialized");
    if(out_ctx_type == ALL)
        throw runtime_error("IDFT transform output data may be only real or comlpex");

    gsl_fft_complex_inverse(S.data(), 1, N, complex_wavetable.get(),complex_workspace.get());

    if(out_ctx_type == REAL){
        std::vector<double> out(N);
        for(size_t i = 0; i < N; i++)
            out[i] = S.data()[2*i];
        S = move(out);
    }
}