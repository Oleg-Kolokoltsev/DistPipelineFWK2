//
// Created by morrigan on 2/9/18.
//

#ifndef DISTPIPELINEFWK_DFRFT_CUBLAS_H
#define DISTPIPELINEFWK_DFRFT_CUBLAS_H

#include <iostream>

#include "dfrft_gsl.h"
#include "config.h"

#ifdef HAVE_CUBLAS_V2_H
    #include "dfrft_cublas.h"
    #include "cublas_v2.h"
#endif

class DFrFTCuBLAS: public DFrFTGSL{
public:

    using tBase = DFrFTGSL;

    DFrFTCuBLAS();
    ~DFrFTCuBLAS();

    void create_basis(size_t);
    virtual void init_transform(double a);
    virtual void dfrft(std::vector<double>& );

private:
    bool cuda_safe(int, const std::string&, std::ostream&);

private:
    /*
     * In the constructor we search for CUDA compatible device,
     * and if it is not found, all functions will repeat the
     * DFrFTGSL functionality.
     */
    bool device_found;

    /*
     * cuBLAS specific entries
     */
#ifdef HAVE_CUBLAS_V2_H
    std::shared_ptr<cublasContext> handle;

    cuDoubleComplex * cuDevEc;
    //double * cuDevEc;
#endif

};


#endif //DISTPIPELINEFWK_DFRFT_CUBLAS_H
