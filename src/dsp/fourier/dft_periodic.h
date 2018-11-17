//
// Created by morrigan on 18/01/18.
//

#ifndef DISTPIPELINEFWK_CONTEXT_FFT_H
#define DISTPIPELINEFWK_CONTEXT_FFT_H

#include <vector>
#include <memory>

#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>
#include <gsl/gsl_fft_complex.h>

//TODO: add parallel integration (do not forget about generic thread safety)

class DFT{
    using tWavetable = std::shared_ptr<gsl_fft_real_wavetable>;
    using tWorkspace = std::shared_ptr<gsl_fft_real_workspace>;
    using tPtrComplexWavetable = std::shared_ptr<gsl_fft_complex_wavetable>;
    using tPtrComplexWorkspace = std::shared_ptr<gsl_fft_complex_workspace>;

public:
    /*
     * If the input data is real, for example a real signal s(t)
     * and it is only required to apply a direct Fourier transform,
     * it is possible to initialize only REAL real_workspace.
     *
     * If it is required to make an inverse transform, the input data
     * will be complex, so it is required to initialize a complex context.
     *
     * If number of points does not change, the workspace and wavetable
     * contexts can be reused without reinitialization and it's storage
     * is the main purpose of the DFT class.
     */
    enum CONTEXT_TYPE{REAL, COMPLEX, ALL};
    void initialize(size_t, CONTEXT_TYPE t);
    bool is_initialized(size_t, CONTEXT_TYPE t);

    /*
     * Direct Fourier transform of real data will return a complex data,
     * so the input vector size will be doubled. Before using this function
     * the REAL context has to be initialized.
     */
    void dft(std::vector<double>&, CONTEXT_TYPE);

    /*
     * Inverse fourier transform is applied to complex data. In a generic case,
     * the result is also a complex vector of the same size. If input data
     * is an image of a real signal, the output vector will have all
     * imaginary parts zero or nearly zero.
     */
    void ift(std::vector<double>&, CONTEXT_TYPE);

protected:
    tWavetable real_wavetable;
    tWorkspace real_workspace;
    tPtrComplexWavetable complex_wavetable;
    tPtrComplexWorkspace complex_workspace;
};

#endif //DISTPIPELINEFWK_CONTEXT_FFT_H
