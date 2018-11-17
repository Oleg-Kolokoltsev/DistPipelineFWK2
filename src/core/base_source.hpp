//
// Created by morrigan on 13/01/18.
//

#ifndef DISTPIPELINEFWK_BASE_SOURCE_H
#define DISTPIPELINEFWK_BASE_SOURCE_H

#include "base_node.hpp"

/*
 * A simple source class does not store the source state between data acquisition
 * calls and will just call the specified func_acquire each time when receives
 * an AQUIRE message. In the case when it is required to store some data
 * between func_acquire calls, the derived from the BaseNode new Source class shell
 * override it's main loop function.
 * todo: is it a good solution for the cmd message passing case?
 */

template<typename tOut>
class BaseSource : public BaseNode<BaseMessage>{
    static_assert(std::is_base_of<BaseMessage, tOut>(),
                  "tOut shell be derived from BaseMessage class");

public:
    using tBase = BaseNode<BaseMessage>;
    using tPtrOut = std::shared_ptr<tOut>;
    using tPtrIn = tBase::tPtrIn;
    using tPtrNext = std::shared_ptr<BaseNode<tOut>>;

protected:
    friend class NodeFactory;
    BaseSource(std::function<tPtrOut()> func_acquire,
               QUEUE_POLICY pol = QUEUE_POLICY::DROP, std::string name = "BaseSource"):
            tBase(name),
            func_acquire{func_acquire},
            pol(pol) {}

    virtual bool process_usr_msg(tPtrIn&& msg){
        if(next){
            if(msg->cmd == MSG_CMD::ACQUIRE) {
                tPtrOut out_msg = func_acquire();
                out_msg->sent_from = this->uid;
                return next->put(move(out_msg), this->uid, pol);
            }
        }else{
            std::cerr << name << " broken pipe" << std::endl;
            return false;
        }
    };

public:
    void set_target(tPtrNext target){next = target;}

protected:
    // function to be called each time AQUIRE message was received
    std::function<tPtrOut()> func_acquire;

    // next node in the processing chain
    tPtrNext next;

    // the policy to be used when this node is sending
    // a new message to the "next" node, can be WAIT or DROP
    QUEUE_POLICY pol;
};

#endif //DISTPIPELINEFWK_BASE_SOURCE_H
