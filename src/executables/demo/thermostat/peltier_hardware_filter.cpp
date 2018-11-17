//
// Created by morrigan on 6/23/18.
//

#include "peltier_hardware_filter.h"

#include <string>

using namespace std;

// todo: BaseFilter<PIDMsgOut, SerialOutPkt>;

PeltierHardwareFilter::tPtrOut
PeltierHardwareFilter::internal_filter(tPtrIn&& msg){

    /*
     * This code is not a good example. write_some function
     * will definitely send just one byte (or will fail with an exception),
     * but if you like to send many bytes - much more generic code is necessary.
     */
    if(msg->cmd == MSG_CMD::ACQUIRE) return nullptr;

    char a = (msg->val);
    try {
        port->write_some(boost::asio::buffer(&a, 1));
    }catch(...){
        cerr << "PeltierHardwareFilter::internal_filter failed to send data to serial port" << endl;
    }

    return nullptr;
}
