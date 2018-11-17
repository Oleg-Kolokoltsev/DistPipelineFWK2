//
// Created by morrigan on 2/4/18.
//

#ifndef DISTPIPELINEFWK_POWER_FILTER_H
#define DISTPIPELINEFWK_POWER_FILTER_H

#include <vector>
#include <memory>
#include <complex>

#include "data_packet_types.h"
#include "base_filter.hpp"

/*
 * Depending on the PowerFilter::MODE switch the output can be |s(k)|, |s^2(k)|, 
 * or 10*Log10(s^2(k)/<s^2(k)>), where <s^2(k)> is a total signal power.
 *
 * This filter is able to work with real or complex input data.
 */

template<typename tIn, typename tOut>
class PowerFilter : public BaseFilter<tIn, tOut>{
    static constexpr bool in_cmplx = std::is_base_of<ComplexSignalPkt, tIn>();
    static constexpr bool in_ok = std::is_base_of<RealSignalPkt, tIn>() || in_cmplx;

    static_assert(in_ok, "tIn shell be derived from RealSignalPkt or ComplexSignalPkt");
    static_assert(std::is_base_of<RealSignalPkt, tOut>(), "tOut shell be derived from RealSignalPkt class, power is always real");

public:
    using tBase = BaseFilter<tIn, tOut>;
    using tPtrOut = typename tBase::tPtrOut;
    using tPtrIn = typename tBase::tPtrIn;

    enum MODE {MAG, POW, POW_DB};

private:
    // real input data
    tPtrOut proc_r(tPtrIn &&in_msg) {
        auto out_msg = tPtrOut(new tOut);
        out_msg->data = std::move(in_msg->data);

        double p = mode == MODE::MAG ? 1.0 : 2.0;
        double sum = 0.0;
        for (int i = 0; i < in_msg->data.size(); i++) {
            out_msg->data[i] = pow(out_msg->data[i], p);
            sum += out_msg->data[i];
        }

        if(mode == MODE::POW_DB)
            log_pow(sum, out_msg);

        return out_msg;
    }

    // complex input data
    tPtrOut proc_c(tPtrIn &&in_msg) {
        auto N = in_msg->data.size() / 2;
        auto out_msg = tPtrOut(new tOut);

        out_msg->data.resize(N);
        double p = mode == MODE::MAG ? 1.0 : 2.0;
        double sum = 0.0;
        for (int i = 0; i < N; i++) {
            std::complex<double> c(in_msg->data[2 * i], in_msg->data[2 * i + 1]);
            out_msg->data[i] = pow(abs(c), p);
            sum += out_msg->data[i];
        }

        if(mode == MODE::POW_DB)
            log_pow(sum, out_msg);

        return out_msg;
    }

    static void log_pow(double pow, tPtrOut& out_msg){
        if(pow == 0) return;
        auto N = out_msg->data.size();
        for (int i = 0; i < N; i++)
            out_msg->data[i] = 10.0*log10(out_msg->data[i]/pow);
    }

protected:
    virtual tPtrOut internal_filter(tPtrIn&& in_msg){
        if(in_cmplx) return proc_c(std::move(in_msg));
        return proc_r(std::move(in_msg));
    }

public:
    PowerFilter(MODE mode = MODE::MAG, QUEUE_POLICY pol = QUEUE_POLICY::WAIT, std::string name = "PowerFilter"):
            tBase(nullptr, pol, name), mode{mode}{}

private:
    MODE mode;
};

#endif //DISTPIPELINEFWK_POWER_FILTER_H
