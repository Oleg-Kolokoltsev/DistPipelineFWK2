//
// Created by morrigan on 17/12/17.
//

#ifndef DISTPIPELINEFWK_FILTER_FFT_H
#define DISTPIPELINEFWK_FILTER_FFT_H

#include <vector>
#include <memory>

#include "data_packet_types.h"
#include "base_filter.hpp"
#include "dft_periodic.h"

template<typename tIn, typename tOut>
class DFTFilter : public BaseFilter<tIn, tOut>, public DFT{
    constexpr static bool is_real_input = std::is_base_of<RealSignalPkt, tIn>();
    static_assert(std::is_base_of<ComplexSignalPkt, tIn>() || is_real_input,
                  "tIn shell be derived from RealSignalPkt or ComplexSignalPkt classes");
    static_assert(std::is_base_of<ComplexSignalPkt, tOut>(),
                  "tOut shell be derived from ComplexSignalPkt class");

public:
    using tBase = BaseFilter<tIn, tOut>;
    using tPtrOut = typename tBase::tPtrOut;
    using tPtrIn = typename tBase::tPtrIn;

protected:
    friend class NodeFactory;
    DFTFilter(QUEUE_POLICY pol = QUEUE_POLICY::DROP, std::string name = "DFTFilter"):
            tBase(nullptr, pol, name){}

    virtual tPtrOut internal_filter(tPtrIn&& in_msg){

        CONTEXT_TYPE ctx_type = is_real_input ? REAL : COMPLEX;
        size_t N;
        if(is_real_input){
                N = in_msg->data.size();
        }else{
                if(in_msg->data.size() % 2 != 0)
                        throw std::runtime_error("complex signal data can't have odd length");
                N = in_msg->data.size()/2;
        }


        if(!is_initialized(N, ctx_type))
                initialize(N, ctx_type);

        tPtrOut out_msg(new typename tPtrOut::element_type);

        /*
         * The transform result is written into the vector of in_msg.
         * If input context type is REAL, the size of in_msg->data will be doubled to
         * store complex numbers. If the context is COMPLEX, than original vector
         * size will not change.
         */
        dft(in_msg->data, ctx_type);
        out_msg->data = std::move(in_msg->data);
        return out_msg;
    }
};

#endif //DISTPIPELINEFWK_FILTER_FFT_H

