#include <vector>
#include <cmath>
#include <random>
#include <complex>

#include "base_splitter.hpp"
#include "base_source.hpp"
#include "dft_filter.hpp"
#include "quadrature_filter.hpp"

#include "simplescope.h"
#include "window.hpp"


using tSource = BaseSource<RealSignalPkt>;
using tSplitter = BaseSplitter<RealSignalPkt>;
using tFilterQuadrature = FilterQuadrature<RealSignalPkt>;
using tDevice = GenOscDevice<RealSignalPkt>;

const size_t N = 1024; //number of signal points
std::default_random_engine generator; //random number generator

tSource::tPtrOut src_proc(){
    tSource::tPtrOut p_msg(new tSource::tPtrOut::element_type);

    p_msg->data.resize(N);

    std::normal_distribution<double> distribution(0,0.02);

    for(double t = 0; t < p_msg->data.size(); t++) {
        p_msg->data[(int)t] = 5*cos(t/2.0)*exp(-0.005*t) + distribution(generator);
    }

    return p_msg;
}

int main(int argc, char** argv){

    auto src = NodeFactory::create<tSource>(src_proc);
    auto split = NodeFactory::create<tSplitter>();
    auto quadrature_filter = NodeFactory::create<tFilterQuadrature>();
    auto dev_osc_1 = NodeFactory::create<tDevice>();
    auto dev_osc_2 = NodeFactory::create<tDevice>();

    src->set_target(split);
    split->add_target(quadrature_filter);
    quadrature_filter->set_target(dev_osc_1);
    split->add_target(dev_osc_2);

    //create windows and renderers (Oscilloscope)
    Window w1(unique_ptr<SimpleScope>(new SimpleScope(dev_osc_1, 50)));
    w1.create_window();

    Window w2(unique_ptr<SimpleScope>(new SimpleScope(dev_osc_2, 50)));
    w2.create_window();

    while(1){
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::DROP);
        if(!w1.is_running() || !w2.is_running()) break;
        this_thread::sleep_for(chrono::milliseconds(40));
    }

    return 0;
}