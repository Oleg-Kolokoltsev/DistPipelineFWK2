//
// Created by morrigan on 11/9/19.
//

/*
 * It is supposed here that each block contains data points acquired by
 * 10-bit ADC and their timestamps (local timer counter).
 *
 * Data format has the next form:
 * val-ts-val-ts-...val-ts
 *
 * where val is 2-byte integer ranging from 0 to 1024
 * and ts is a 4-byte integer.
 *
 * The block size is multiple of 6, so everithing has to be on it's places.
 * In the case if UDPSource packet size is smaller than a real UDP packet size,
 * the val-ts bytes may be mixed. In this case the packet splitting is required
 * would produce much more additional coding and assumptions.
 */

#include "src_udp.hpp"
#include "base_splitter.hpp"
#include "base_filter.hpp"
#include "simplescope.h"
#include "window.hpp"

using namespace std;

using tSource = UDPSource<1024>;
using tSplitter = BaseSplitter<UDPOutPkt>;
using tFilter = BaseFilter<UDPOutPkt, RealSignalPkt>;
using tDevice = GenOscDevice<RealSignalPkt>;

tFilter::tPtrOut filter_proc_dt(tFilter::tPtrIn && msg){
    tFilter::tPtrOut p_msg(new tFilter::tPtrOut::element_type);

    int len = msg->block.size()/6;
    p_msg->data.resize(len-1);

    uint32_t t0;
    memcpy(&t0, &msg->block.data()[2], 4);


    for(int i = 1; i < len; i++) {
        uint16_t v;
        uint32_t t, dt;
        memcpy(&v, &msg->block.data()[i*6], 2);
        memcpy(&t, &msg->block.data()[i*6+2], 4);
        dt = t - t0;
        t0 = t;
        p_msg->data[i-1] = dt;
    }

    return p_msg;
}

tFilter::tPtrOut filter_proc_adc(tFilter::tPtrIn && msg){
    tFilter::tPtrOut p_msg(new tFilter::tPtrOut::element_type);

    int len = msg->block.size()/6;
    p_msg->data.resize(len);

    for(int i = 0; i < len; i++) {
        uint16_t v;
        memcpy(&v, &msg->block.data()[i*6], 2);
        p_msg->data[i] = v;
    }

    return p_msg;
}

int main(){

    auto src = NodeFactory::create<tSource>("192.168.1.255:1234");
    auto split =  NodeFactory::create<tSplitter>(QUEUE_POLICY::DROP, "Splitter");
    auto filter_dt = NodeFactory::create<tFilter>(filter_proc_dt);
    auto filter_adc = NodeFactory::create<tFilter>(filter_proc_adc);
    auto osc_dt = NodeFactory::create<tDevice>();
    auto osc_adc = NodeFactory::create<tDevice>();

    src->set_target(split);
    split->add_target(filter_dt);
    split->add_target(filter_adc);
    filter_dt->set_target(osc_dt);
    filter_adc->set_target(osc_adc);

    Window w_dt(unique_ptr<SimpleScope>(new SimpleScope(osc_dt, 50)));
    w_dt.create_window();

    Window w_adc(unique_ptr<SimpleScope>(new SimpleScope(osc_adc, 50)));
    w_adc.create_window();

    while(1){
        if(!w_dt.is_running()) break;
        if(!w_adc.is_running()) break;
        this_thread::sleep_for(chrono::milliseconds(40));
    }

    return 0;
}
