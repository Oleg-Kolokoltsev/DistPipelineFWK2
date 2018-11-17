//
// Created by morrigan on 17/12/17.
//

#ifndef DISTPIPELINEFWK_QUADRATURE_FILTER_H
#define DISTPIPELINEFWK_QUADRATURE_FILTER_H

#include <complex>

#include "dft_periodic.h"
#include "data_packet_types.h"
#include "base_filter.hpp"

template<typename tIn>
class FilterQuadrature : public BaseFilter<tIn, tIn>, public DFT{
    static_assert(std::is_base_of<RealSignalPkt, tIn>(),
                  "tIn shell be derived from RealSignalPkt class");

public:
    using tBase = BaseFilter<tIn, tIn>;
    using tPtrOut = typename tBase::tPtrOut;
    using tPtrIn = typename tBase::tPtrIn;

    FilterQuadrature(QUEUE_POLICY pol = QUEUE_POLICY::DROP, std::string name = "FilterQuadrature"):
            tBase(nullptr, pol, name) {}

protected:
    virtual tPtrOut internal_filter(tPtrIn&& in_msg){
        auto N = in_msg->data.size();
        if(!is_initialized(N, ALL)) initialize(N, ALL);

        // todo: remove constant component?

        // store original input signal
        std::vector<double> cpy = in_msg->data;

        // fourier transform result is complex vector of size 2*N
        dft(in_msg->data, REAL);

        // ft is just an alias to in_msg->data
        auto& ft = in_msg->data;

        // hilbert transform
        std::complex<double> I(0,1);
        for(int i = 0; i < N; i++){
            std::complex<double> x(ft[2*i],ft[2*i+1]);

            x = i < N/2 ? x*I : -x*I;

            ft[2*i] = real(x);
            ft[2*i+1] = imag(x);
        }

        // apply inverse Fourier to get shifted signal, ft aka in_msg->data is still complex,
        // but imaginary part of each point is nearly zero
        ift(ft, COMPLEX);

        tPtrOut out_msg(new typename tPtrOut::element_type(*in_msg.get()));
        out_msg->data.resize(N);

        // quadrature magnitude
        for(int i = 0; i < N; i++)
            out_msg->data[i] = sqrt(cpy[i]*cpy[i] + ft[2*i]*ft[2*i] + ft[2*i+1]*ft[2*i+1]);

        return out_msg;
    }

};

#endif //DISTPIPELINEFWK_QUADRATURE_FILTER_H
