//
// Created by morrigan on 2/8/18.
//

#include <vector>
#include <cmath>
#include <random>
#include <complex>

#include "base_splitter.hpp"
#include "base_source.hpp"
#include "dft_filter.hpp"
#include "power_filter.hpp"

#include "simplescope.h"
#include "window.hpp"

using tSource = BaseSource<RealSignalPkt>;
using tDFTFilter = DFTFilter<RealSignalPkt, ComplexSignalPkt>;
using tAbsComplex = PowerFilter<ComplexSignalPkt, RealSignalPkt>;

const size_t N = 1024; //number of signal points

double t_sweep = 0;
tSource::tPtrOut src_proc(){
    tSource::tPtrOut p_msg(new tSource::tPtrOut::element_type);

    p_msg->data.resize(N);

    double w0 = M_PI/N;
    double w_base = w0*N/30;
    double dw = w0*cos(t_sweep);

    //Blackamn window coefficients
    const double a0 = 7938.0/18608.0;
    const double a1 = 9240.0/18608.0;
    const double a2 = 1430.0/18608.0;

    for(double t = 0; t < p_msg->data.size(); t++)
        p_msg->data[(int)t] =
                // plain 1*w_base
                cos((w_base + dw)*t) +
                // windowed 2*w_base
                (a0 - a1*cos(2*M_PI*t/(N-1)) + a2*cos(4*M_PI*t/(N-1)))*cos((2*w_base + dw)*t);

    /*
     * todo: move this to shift theorem example
     * hint to get zero-centered DFT spectra by using shift theorem
     */
    for(int i = 0; i < p_msg->data.size(); i++)
        p_msg->data[i] = p_msg->data[i]*pow(-1.0, i);


    t_sweep += 0.02;

    return p_msg;
}

int main(int argc, char** argv){

    auto src = NodeFactory::create<tSource>(src_proc);
    auto fft_filter = NodeFactory::create<tDFTFilter>();
    auto abs_complex = NodeFactory::create<tAbsComplex>(tAbsComplex::MAG);
    auto dev_osc = NodeFactory::create<GenOscDevice<RealSignalPkt>>();

    src->set_target(fft_filter);
    fft_filter->set_target(abs_complex);
    abs_complex->set_target(dev_osc);

    //create windows and renderers (Oscilloscope)
    Window w1(unique_ptr<SimpleScope>(new SimpleScope(dev_osc, 50)));
    w1.create_window();

    while(1){
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::DROP);
        if(!w1.is_running()) break;
        this_thread::sleep_for(chrono::milliseconds(40));
    }

    return 0;
}
