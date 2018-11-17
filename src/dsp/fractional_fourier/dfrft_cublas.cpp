//
// Created by morrigan on 2/9/18.
//

#include <iostream>
#include <cstring>

#include <gsl/gsl_matrix.h>

#include "dfrft_cublas.h"
#include "dfrft_commons.h"

using namespace std;

//todo: do not forget Fa = nullptr trick
//todo: add decision if it is better than CPU (run test task),  include all that in IDFrFT

//todo: remove this fkn macro
#define IDX2C(i,j,ld) (((i)*(ld))+(j))

#ifdef HAVE_CUBLAS_V2_H

#include <cuda_runtime.h>
DFrFTCuBLAS::DFrFTCuBLAS() : tBase() {

    cout << "Initializing new DFrFTCuBLAS object...\n\n" ;

    int dev_count;
    if(cuda_safe(cudaGetDeviceCount(&dev_count), string(__FILE__), cout)){
        device_found = false;
        return;
    };

    // select device
    // todo: may be conflicts with multiply instances of this class if the same device where selected!
    int n_mult = -1;
    int dev_sel;
    for (int dev = 0; dev < dev_count; dev++) {
        cudaDeviceProp deviceProp;
        cuda_safe(cudaGetDeviceProperties(&deviceProp, dev), string(__FILE__), cout);

        if(dev == 0 && (deviceProp.major == 9999 && deviceProp.minor == 9999)){
            device_found = false;
            return;
        }

        cout << "For device #" << dev << endl;
        cout << "Device name:                " << deviceProp.name << endl;
        cout << "Major revision number:      " << deviceProp.major << endl;
        cout << "Minor revision Number:      " << deviceProp.minor << endl;
        cout << "Total Global Memory:        " << deviceProp.totalGlobalMem << endl;
        cout << "Total shared mem per block: " << deviceProp.sharedMemPerBlock << endl;
        cout << "Total const mem size:       " << deviceProp.totalConstMem << endl;
        cout << "Warp size:                  " << deviceProp.warpSize << endl;
        cout << "Maximum block dimensions:   "
             << deviceProp.maxThreadsDim[0]
             << "x" << deviceProp.maxThreadsDim[1]
             << "x" << deviceProp.maxThreadsDim[2] << endl;
        cout << "Maximum grid dimensions:    "
             << deviceProp.maxGridSize[0]
             << "x" << deviceProp.maxGridSize[1]
             << "x" << deviceProp.maxGridSize[2] << endl;
        cout << "Clock Rate:                 " << deviceProp.clockRate << endl;
        cout << "Number of muliprocessors:   " << deviceProp.multiProcessorCount << endl << endl;

        if(n_mult < deviceProp.multiProcessorCount){
            n_mult = deviceProp.multiProcessorCount;
            dev_sel = dev;
        }
    }

    if(cuda_safe(cudaSetDevice(dev_sel), string(__FILE__), cerr)){
        device_found = false;
        return;
    }

    // ************ get device handler ***********

    cublasContext* h;
    if(cuda_safe(cublasCreate(&h), string(__FILE__), cerr) || !h){
        device_found = false;
        return;
    }

    handle = shared_ptr<cublasContext>(h, [](cublasContext *p){cublasDestroy(p);});
    cout << "Selected device for active DFrFTCuBLAS thread: #" << dev_sel << endl << endl;

    device_found = true;

}

DFrFTCuBLAS::~DFrFTCuBLAS(){
    //an order is important
    if(device_found) {
        cuda_safe(cudaFree(cuDevEc), string(__FILE__), std::cerr);
        handle.reset();
    }
}

void DFrFTCuBLAS::create_basis(size_t N){
    cout << "Creating DFrFT basis..." << endl;
    tBase::create_basis(N);
    if(!device_found) return;

    //todo: kill
    dsp::print_mat_c(tBase::Ec.get());

    //allocate space in GPU

    if(cuda_safe(cudaMalloc(&cuDevEc, N*N*sizeof(double)), string(__FILE__), cerr)) return;

    cuDoubleComplex* a = new cuDoubleComplex[N*N];
    auto mat = gsl_matrix_alloc(N, N);
    gsl_matrix_set_all(mat, 1.0);


    //todo: does not want to work with complex data?
    cout << "sizeof(cuDoubleComplex) = " << sizeof(double) << endl;
    cuda_safe(cublasSetMatrix (N, N, sizeof(cuDoubleComplex), a, N, cuDevEc, N), string(__FILE__), cerr);
    //memset (a, 0, N*N*sizeof(*a));
    //cuda_safe(cublasGetMatrix (N, N, sizeof(*a), cuDevEc, N, a, N), string(__FILE__), cerr);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf ("(%5.2f, %5.2f)", a[(i*N + j)].x, a[(i*N + j)].y);
        }
        printf ("\n");
    }

    cout << endl << endl;

    //for (int j = 0; j < N; j++) {
    //    for (int i = 0; i < N; i++) {
    //        printf ("%5.2f", Ec->data[2*(j*N + i)]);
    //    }
    //    printf ("\n");
    //}


    delete [] a;
    gsl_matrix_free(mat);

}

void DFrFTCuBLAS::init_transform(double a){
    if(!device_found){ tBase::init_transform(a); return;}
    cout << "Transform init with a = " << a << endl;
}

void DFrFTCuBLAS::dfrft(std::vector<double>& data){
    if(!device_found){tBase::dfrft(data); return;}

}



//***********************************************************************************


/*
 * In the case of CUBLAS is not present, this class can behave equivalently
 * to it's base class. So when this .cpp will be compiled, all will work, but with GSL.
 *
 * todo: this is an ad-hook for students, shell be removed in future
 */
#elif HAVE_CUBLAS_V2_H
DFrFTCuBLAS::DFrFTCuBLAS() : tBase() {
cout << "CUDA or cuBLAS SDK not installed" << endl;
    device_found = false;
    return;
}
void DFrFTCuBLAS::create_basis(size_t N){tBase::create_basis(N);}
void DFrFTCuBLAS::init_transform(double a){tBase::init_transform(a);}
void DFrFTCuBLAS::dfrft(std::vector<double>& data){tBase::dfrft(data);}
#endif

//**************** HELPERS *****************
//bool DFrFTCuBLAS::cuda_safe(int err, const std::string& msg, std::ostream& os){
//    if(err) os << "CUDA error: " << msg << endl;
//    return err != cudaSuccess;
//}
