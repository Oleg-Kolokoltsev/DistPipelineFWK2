//
// Created by morrigan on 6/19/18.
//

#ifndef DISTPIPELINEFWK_PELTIER_MODELA_H
#define DISTPIPELINEFWK_PELTIER_MODELA_H

#include <iostream>

#include "peltier_model.h"
#include "base_filter.hpp"
#include "pid_filter.hpp"
#include "messages.h"


class PeltierModelFilter : public BaseFilter<PIDMsgOut, PeltierStateMsg>, PeltierModel{

public:

    using tBase = BaseFilter<PIDMsgOut, PeltierStateMsg>;
    using tPtrIn = tBase::tPtrIn;
    using tPtrOut = tBase::tPtrOut;

protected:
    friend class NodeFactory;
    PeltierModelFilter() : tBase(nullptr, QUEUE_POLICY::DROP, "PeltierModelFilter"), PeltierModel(){};

    virtual void main_loop();
};



#endif //DISTPIPELINEFWK_PELTIER_MODELA_H
