//
// Created by morrigan on 6/18/18.
//

#ifndef DISTPIPELINEFWK_MESSAGES_H
#define DISTPIPELINEFWK_MESSAGES_H

#include <map>

#include "data_packet_types.h"

struct PeltierState{
    PeltierState(){};
    PeltierState(double T_targ, double T_in, double T_out, double E) :
    T_targ{T_targ}, T_in{T_in}, T_out{T_out}, E{E}{};
    double T_targ, T_in, T_out, E;
};

// shell be derived from BaseMessage, and if it is not the compilation assert message appear
struct PeltierStateMsg : public BaseMessage, PeltierState{

    // empty constructor, required to create new messages
    PeltierStateMsg(){}

    // copy constructor, required for message copying,
    // please note the call to base class constructor, it is not empty
    PeltierStateMsg(const PeltierStateMsg& msg) : BaseMessage(msg){
        T_targ = msg.T_targ;
        T_in   = msg.T_in;
        T_out  = msg.T_out;
        E      = msg.E;
    }
};

#endif //DISTPIPELINEFWK_MESSAGES_H
