//
// Created by morrigan on 17/12/17.
//

#ifndef DISTPIPELINEFWK_DATA_PACKET_TYPES_H
#define DISTPIPELINEFWK_DATA_PACKET_TYPES_H

#include <memory>
#include <exception>
#include <vector>

#include "cmd_data_types.h"
#include "node_factory.hpp"

struct BaseMessage {
    /*
     * An empty constructor, the message fields has to be initialized
     * in the new object manually.
     */
    BaseMessage() {
        uid = NodeFactory::generate_random_uid();
    }

    /*
     * Simplified constructor for command messages.
     */
    BaseMessage(MSG_CMD cmd) : cmd{cmd} {
        uid = NodeFactory::generate_random_uid();
    }

    /*
     * The tBaseMessage and all derived classes has to
     * implement the copy constructor. DO NOT FORGET to call parent
     * constructor (this one) as usual.
     */
    BaseMessage(const BaseMessage& msg){
        init_attached_data(msg);
    }

    /*
     * Virtual destructor is needed for dynamic downcast of BaseMessage to it's child type
     * using dynamic_pointer_cast<DerivedMsg>(ptr), where 'ptr' has shared_pointer<MaseMessage>
     * type that was previously upcasted.
     */
    virtual ~BaseMessage(){}

    /*
     * In the case when newly created message shell keep attached data of the
     * other message, and they have different data types - use this function.
     * It is especially important to preserve the message UID.
     */

    void init_attached_data(const BaseMessage& msg){
        this->cmd = msg.cmd;
        this->user_data = msg.user_data ? msg.user_data->clone() : nullptr;
        this->uid = msg.uid;
    }

    /*
     * Each message have a unique identifier. It is used in BaseSyncJoin.
     * When the message is splitted, it's uid is cloned. So that BaseSyncJoin
     * could identify joinable messages which came from different channels.
     */
    unsigned int get_uid(){
        return uid;
    }

    /*
     * A command attached to a message. See cmd_data_types.h for
     * details.
     */
    MSG_CMD cmd = MSG_CMD::NONE;

    /*
     * Any specific data attached to this message. Usually used with
     * a specific command that can be recognized by the target of
     * this data. It can be used as a command data for any filter. For example,
     * the filter is already created, but during the program execution
     * some of it's parameters need an update.
     */
    std::shared_ptr<ICloneable> user_data = nullptr;

    /*
     * Each time the message passes through any filter,
     * the data attached to it's predecesor will be
     * transferred to the next message. If you want to
     * flush this data somewhere in the chain - set false here.
     * (see base_filter code for reference)
     */
    bool keep_prev_attached_data = true;


    /*
     * Each time you send the message it is required to
     * store here an UID of the node from where it was sent.
     * By default is is zero, that means that the message was
     * sent from outside of the framework (from main function for example).
     */
    unsigned int sent_from = 0;

private:
    /*
     * Each message has it's unique ID when created. If we clone the message,
     * for example in the BaseSplitter, it's UID is also cloned.
     */
    unsigned int uid;
};

// custom tData shell have a copy constructor, so there is no sence to use this
// class with custom data structures, however a lot's of STL classes
// hav their copy constructors well implemented
template <typename tData>
struct GenericDataPkt : public BaseMessage{
    virtual ~GenericDataPkt(){};

    //empty constructor
    GenericDataPkt(){};

    //copy constructor
    GenericDataPkt(const GenericDataPkt& msg) : BaseMessage(msg){
        this->val = msg.val;
    };

    // signal data
    tData val;
};

struct RealSignalPkt : public BaseMessage{
    //empty constructor
    RealSignalPkt(){};

    //copy constructor
    RealSignalPkt(const RealSignalPkt& msg) : BaseMessage(msg){
        this->data = msg.data;
    };

    // signal data
    std::vector<double> data;
};


struct ComplexSignalPkt : public BaseMessage{
    //empty constructor
    ComplexSignalPkt(){}

    //copy constructor
    ComplexSignalPkt(const ComplexSignalPkt& msg) : BaseMessage(msg){
        this->data = msg.data;
    }

    //the C-style complex number array: [ReImReImReIm....]
    //it's size is 2*N
    std::vector<double> data;
};

#endif //DISTPIPELINEFWK_DATA_PACKET_TYPES_H