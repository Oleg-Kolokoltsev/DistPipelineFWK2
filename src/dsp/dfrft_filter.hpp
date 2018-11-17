//
// Created by morrigan on 19/01/18.
//

#ifndef DISTPIPELINEFWK_DFRFT_HPP
#define DISTPIPELINEFWK_DFRFT_HPP

#include <iostream>
#include <gsl/gsl_matrix.h>

#include "base_filter.hpp"
#include "dfrft_gsl.h"

template<typename tIn, typename tOut>
class DFrFTFilter : public BaseFilter<tIn, tOut>, public DFrFTGSL{
    static_assert(std::is_base_of<RealSignalPkt, tIn>(),
                  "tIn shell be derived from RealSignalPkt class");
    static_assert(std::is_base_of<ComplexSignalPkt, tOut>(),
                  "tOut shell be derived from ComplexSignalPkt class");

public:
    using tBase = BaseFilter<tIn, tOut>;
    using tPtrOut = typename tBase::tPtrOut;
    using tPtrIn = typename tBase::tPtrIn;

    // COMMAND MESSAGES

    /*
     * During continuous FrFT application it can be required to
     * change the FrFT angle parameter 'a' on the fly. For example, during
     * calibration. Any node can send tUsrCmdRotate message to DFrFTFilter.
     */
    struct tUsrCmdRotate : public ICloneable{

        // simple constructor
        tUsrCmdRotate(double a) : a{a} {};

        // clone function
        virtual tPtrCloneable clone() const {
            return std::shared_ptr<tUsrCmdRotate>(new tUsrCmdRotate(a));
        }

        // apply when arrived
        virtual void apply(CommandNode* ptr){
            auto filter = dynamic_cast<DFrFTFilter<tIn, tOut> *>(ptr);
            if (!filter) return;
            // apply only if current 'a' is not the same
            if (filter->a != a) {
                // store new 'a' - angle
                filter->a = a;
                // invalidate current transformation matrix, it
                // will be recalculated in the DFrFTFilter::internal_filter function
                filter->Fa = nullptr;
            }
        };

        // the transform angle parameter
        double a = 0.0;
    };

public:

protected:
    friend class NodeFactory;
    DFrFTFilter(QUEUE_POLICY pol = QUEUE_POLICY::DROP, std::string name = "DFrFTFilter"):
            tBase(nullptr, pol, name){}

    virtual tPtrOut internal_filter(tPtrIn&& msg){
        // apply any user command if present in the message
        if(msg->cmd == MSG_CMD::USER && msg->user_data)
            msg->user_data->apply(this);

        // check that message contain any data (it can be empty if a command was sent)
        auto N = msg->data.size();
        if(N == 0) return nullptr;

        if(!Ec || Ec->size1 != N) create_basis(N);
        if(!Fa) init_transform(a);

        tPtrOut out_msg(new typename tPtrOut::element_type);
        dfrft(msg->data);
        out_msg->data = std::move(msg->data);
        return out_msg;
    }

private:
    // current FrFT angle parameter, can be changed by sending tUsrCmdRotate struct
    // in a MSG_CMD::USER message
    double a = 0.0;
};

#endif //DISTPIPELINEFWK_DFRFT_HPP
