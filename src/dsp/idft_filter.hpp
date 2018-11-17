//
// Created by morrigan on 9/28/18.
//

#ifndef DISTPIPELINEFWK_IDFT_FILTER_H
#define DISTPIPELINEFWK_IDFT_FILTER_H

#include <vector>
#include <memory>

#include "data_packet_types.h"
#include "base_filter.hpp"
#include "dft_periodic.h"

template<typename tIn, typename tOut>
class IDFTFilter : public BaseFilter<tIn, tOut>, public DFT{
    constexpr static bool is_real_output = std::is_base_of<RealSignalPkt, tOut>();
    static_assert(std::is_base_of<ComplexSignalPkt, tOut>() || is_real_output,
                  "tOut shell be derived from RealSignalPkt or ComplexSignalPkt classes");
    static_assert(std::is_base_of<ComplexSignalPkt, tIn>(),
                  "tIn shell be derived from ComplexSignalPkt class");

public:
    using tBase = BaseFilter<tIn, tOut>;
    using tPtrOut = typename tBase::tPtrOut;
    using tPtrIn = typename tBase::tPtrIn;

protected:
    friend class NodeFactory;
    IDFTFilter(QUEUE_POLICY pol = QUEUE_POLICY::DROP, std::string name = "IDFTFilter"):
            tBase(nullptr, pol, name){}

    virtual tPtrOut internal_filter(tPtrIn&& in_msg){

        if(in_msg->data.size() % 2 != 0)
            throw std::runtime_error("input complex signal can't have odd length");

        size_t N = in_msg->data.size()/2;
        if(!is_initialized(N, COMPLEX))
            initialize(N, COMPLEX);

        tPtrOut out_msg(new typename tPtrOut::element_type);

        /*
         * The transform result is written into the vector of in_msg.
         * If output context type is REAL, the size of in_msg->data will be twice smaller and will
         * contain only real part of the signal obtained.
         */
        CONTEXT_TYPE ctx_type = is_real_output ? REAL : COMPLEX;
        ift(in_msg->data, ctx_type);
        out_msg->data = std::move(in_msg->data);
        return out_msg;
    }
};

#endif //DISTPIPELINEFWK_IDFT_FILTER_H
