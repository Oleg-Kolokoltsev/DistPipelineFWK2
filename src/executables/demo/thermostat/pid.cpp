//
// Created by morrigan on 6/14/18.
//

#include <iostream>
#include <chrono>

#include "base_source.hpp"
#include "base_filter.hpp"
#include "pid_filter.hpp"
#include "peltier_model_filter.h"
#include "sys_monitor_filter.h"
#include "peltier_hardware_filter.h"
#include "hw_state_tracker.h"

#include "simplescope.h"
#include "window.hpp"

using namespace std;
using namespace chrono;

//#define EXPERIMENT 1


int main(int argc, char** argv){

    using tOsc = GenOscDevice<RealSignalPkt>;

    // DSP
    auto sys_monitor = NodeFactory::create<SysMonitorFilter>();
    auto pid_filter = NodeFactory::create<PIDFilter>(QUEUE_POLICY::WAIT);

    // devices
    auto err_osc = NodeFactory::create<tOsc>("T_in");
    auto u_osc = NodeFactory::create<tOsc>("Source Voltage");


#ifdef EXPERIMENT
    auto hw_peltier = NodeFactory::create<PeltierHardwareFilter>("/dev/ttyACM0");
    auto hw_decoder = NodeFactory::create<HWStateTracker>();
    hw_peltier->set_target(hw_decoder);
    hw_decoder->set_target(sys_monitor);
    pid_filter->set_target(hw_peltier);
#else
    auto software_peltier = NodeFactory::create<PeltierModelFilter>();
    software_peltier->set_target(sys_monitor);
    pid_filter->set_target(software_peltier);
#endif

    sys_monitor->PIDController = pid_filter;
    sys_monitor->scope_Err = err_osc;
    sys_monitor->scope_U = u_osc;

    //create windows and renderers (Oscilloscope)
    Window w_Err(unique_ptr<SimpleScope>(new SimpleScope(err_osc, 50)));
    w_Err.create_window();

    Window w_U(unique_ptr<SimpleScope>(new SimpleScope(u_osc, 50)));
    w_U.create_window();

    while(1) {
        if(!w_Err.is_running()) break;
        if(!w_U.is_running()) break;

        this_thread::sleep_for(chrono::milliseconds(40));
    }

}
