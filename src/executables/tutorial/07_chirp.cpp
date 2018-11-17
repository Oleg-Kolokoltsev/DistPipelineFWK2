#include <iostream>
#include <random>
#include <complex>

#include "base_source.hpp"
#include "base_splitter.hpp"
#include "dft_filter.hpp"
#include "dfrft_filter.hpp"
#include "power_filter.hpp"

#include "simplescope.h"
#include "window.hpp"


const size_t N = 300; //number of signal points
std::default_random_engine generator;

using tSource = BaseSource<RealSignalPkt>;
using tDevice = GenOscDevice<RealSignalPkt>;
using tSplitter = BaseSplitter<RealSignalPkt>;
using tDFrFTFilter = DFrFTFilter<RealSignalPkt, ComplexSignalPkt>;
using tPowerFilter = PowerFilter<ComplexSignalPkt, RealSignalPkt>;

/*
 * chirp source function
 */
tSource::tPtrOut src_proc(){
    tSource::tPtrOut p_msg(new tSource::tPtrOut::element_type);

    p_msg->data.resize(N);

    std::normal_distribution<double> distribution(0.0, M_PI*0.2);

    for(double i = 0; i < N; i++) {
        double t = (i - N/2)/60.0;
        p_msg->data[i] = cos(t*t + 2*t + 0.1*distribution(generator));
    }

    return p_msg;
}


int main(int argc, char** argv){

    auto src = NodeFactory::create<tSource>(src_proc);
    auto split = NodeFactory::create<tSplitter>();
    auto dfrft_filter = NodeFactory::create<tDFrFTFilter>();
    auto power_filter = NodeFactory::create<tPowerFilter>(tPowerFilter::MAG);
    auto dev_osc_1 = NodeFactory::create<tDevice>();
    auto dev_osc_2 = NodeFactory::create<tDevice>();

    src->set_target(split);
    split->add_target(dfrft_filter);
    dfrft_filter->set_target(power_filter);
    power_filter->set_target(dev_osc_1);
    split->add_target(dev_osc_2);

    //create renderers and windows (Oscilloscope)
    Window w1(unique_ptr<SimpleScope>(new SimpleScope(dev_osc_1, 50)));
    Window w2(unique_ptr<SimpleScope>(new SimpleScope(dev_osc_2, 50)));

    w1.create_window();
    w2.create_window();

    using tRotate = tDFrFTFilter::tUsrCmdRotate;
    using tPtrRotate = std::shared_ptr<tRotate>;

    double a = 0.0;
    while(1){
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::DROP);
        dfrft_filter->put(MSG_CMD::USER, 0, QUEUE_POLICY::DROP, move(tPtrRotate(new tRotate(a))));
        a += 0.01;

        if(!w1.is_running() || !w2.is_running()) break;
        this_thread::sleep_for(chrono::milliseconds(40));
    }

    return 0;
}