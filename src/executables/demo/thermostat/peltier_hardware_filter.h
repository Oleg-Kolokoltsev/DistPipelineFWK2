//
// Created by morrigan on 6/23/18.
//

#ifndef DISTPIPELINEFWK_PELTIER_HARDWARE_FILTER_H
#define DISTPIPELINEFWK_PELTIER_HARDWARE_FILTER_H

#include <iostream>

#include "base_filter.hpp"
#include "pid_filter.hpp"
#include "messages.h"

#include "src_serial.hpp"

class PeltierHardwareFilter : public SerialPortSRC<PIDMsgOut /*tIn*/, 1024> {

public:

    using tBase = SerialPortSRC<PIDMsgOut /*tIn*/, 1024>;
    using tPtrIn = tBase::tPtrIn;
    using tPtrOut = tBase::tPtrOut;

protected:
    friend class NodeFactory;
    PeltierHardwareFilter(std::string dev_name) : tBase(dev_name){};

    virtual tPtrOut internal_filter(tPtrIn&&);
};

#endif //DISTPIPELINEFWK_PELTIER_HARDWARE_FILTER_H
