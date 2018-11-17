//
// Created by morrigan on 1/31/17.
//

#include <gsl/gsl_interp.h>
#include <base_splitter.hpp>

#include "base_filter.hpp"
#include "quadrature_filter.hpp"

#include "simplescope.h"
#include "window.hpp"

#include "scpi_cli.hpp"
#include "ieee754_1985.hpp"

// todo: move to plotscope
/*struct tMySignalPkt : public RealSignalPkt{
    //empty constructor
    tMySignalPkt(){}

    //copy constructor
    tMySignalPkt(const tMySignalPkt& pkt) : RealSignalPkt(pkt){
        dt = pkt.dt;
    }

    double dt; //time between neighbour sampling points
};*/

using tFilterBinFloat = BaseFilter<BIN_PKT, RealSignalPkt>;
using tSplitter = BaseSplitter<RealSignalPkt>;
using tFilterQuadrature = FilterQuadrature<RealSignalPkt>;
using tDevice = GenOscDevice<RealSignalPkt>;

//filter converting binary data to float and then cast to double
tFilterBinFloat::tPtrOut bin_float_proc(tFilterBinFloat::tPtrIn&& in_msg){
    tFilterBinFloat::tPtrOut out_msg(new tFilterBinFloat::tPtrOut::element_type);

    out_msg->data.resize(in_msg->bin_frame.size()/4);
    for(int i = 0; i < in_msg->bin_frame.size()/4; i++){
        out_msg->data[i] = (double)ieee754_1985_to_float(&in_msg->bin_frame.data()[i*4]);
    }
    //double dt = in_msg->measure_time/(double) out_msg->data.size();
    cout << out_msg->data.size() << "pts " << "in " << in_msg->t_adq << "ms" << endl;
    return out_msg;
};

int main(int argc, char** argv){

    // create and run all nodes (source, filters, devices)
    auto cli = NodeFactory::create<SCPIClient>("192.168.0.100","5000");
    auto f_bin_float = NodeFactory::create<tFilterBinFloat>(bin_float_proc);
    auto split = NodeFactory::create<tSplitter>();
    auto f_quadrature = NodeFactory::create<tFilterQuadrature>();
    auto osc_dev_1 = NodeFactory::create<tDevice>("Signal");
    auto osc_dev_2 = NodeFactory::create<tDevice>("Quadrature Detector");

    // setup filtering graph
    cli->set_target(f_bin_float);
    f_bin_float->set_target(split);
    split->add_target(f_quadrature);
    f_quadrature->set_target(osc_dev_1);
    split->add_target(osc_dev_2);

    // create two oscilloscopes
    Window w1(unique_ptr<SimpleScope>(new SimpleScope(osc_dev_1, 140)));
    Window w2(unique_ptr<SimpleScope>(new SimpleScope(osc_dev_2, 140)));
    
    w1.create_window();
    w2.create_window();
    while(1){
        if(!w1.is_running() || !w2.is_running()) break;
    }

    return 0;
};