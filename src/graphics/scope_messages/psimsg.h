//
// Created by morrigan on 11/7/18.
//

#ifndef DISTPIPELINEFWK_PSIMSG_H
#define DISTPIPELINEFWK_PSIMSG_H

#include <string>
#include <stdexcept>

#include <allegro5/events.h>

#include "data_packet_types.h"

/*
 * An interface for all PlotScope messages.
 */

struct PSIMsg : public BaseMessage{

    // empty constructor
    PSIMsg(){}

    // copy constructor is required to copy the BaseMesage data
    PSIMsg(const PSIMsg& msg) : BaseMessage(msg){}

    // an empty stub function shell be never called
    virtual void get_data(std::string&) const {
        throw std::runtime_error("PSIMsg::get_data shell be never called directly");
    }

    // TODO: has to keep in memory current plot specific state that does not depend from the data (downcast another object !?!!?)
    // TODO: this data depends from the message type but does not depend from the message data!!!
    // TODO: BUT plotscope do not know nothing about message type, it uses an interface
    //virtual void msg_proc(const ALLEGRO_EVENT& ev, std::string&) const {};
};

#endif //DISTPIPELINEFWK_PSIMSG_H
