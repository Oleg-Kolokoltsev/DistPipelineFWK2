//
// Created by morrigan on 9/27/18.
//

#ifndef DISTPIPELINEFWK_BASE_SYNC_JOIN_H
#define DISTPIPELINEFWK_BASE_SYNC_JOIN_H

#include <functional>
#include <deque>
#include <list>
#include <algorithm>
#include <set>

#include "base_node.hpp"

template<typename tOut>
class BaseSyncJoin : public BaseNode<BaseMessage>{

    static_assert(std::is_base_of<BaseMessage, tOut>(),
                  "tOut shell be derived from BaseMessage class");

public:
    using tBase = BaseNode<BaseMessage>;
    using tPtrIn = tBase::tPtrIn;
    using tPtrOut = std::shared_ptr<tOut>;
    using tMsg = std::shared_ptr<BaseMessage>;
    using tPtrMsgBlock = std::shared_ptr<std::map<unsigned int, tMsg>>;
    using tFuncJoin = std::function<tPtrOut(tPtrMsgBlock&&)>;
    using MsgDeque = std::deque<tPtrIn>;
    using tPtrNext = std::shared_ptr<BaseNode<tOut>>;

public:
    BaseSyncJoin(tFuncJoin func_join, QUEUE_POLICY pol = QUEUE_POLICY::DROP, std::string name = "BaseSyncJoin")
    : tBase(name), func_join{func_join}, pol{pol} {};

    void reg_source_uid(unsigned int src_uid){ src_list.push_back(src_uid); }

    void set_target(tPtrNext target){ next = target; }

    /*
     * Class BaseSyncJoin can receive messages of any type derived from BaseMessage,
     * however, it can't be casted to BaseNode<CustomMsgType> automatically - it is
     * correct here but incorrect in a generic case.
     */
    template<typename tCustomMessage>
    static std::shared_ptr<BaseNode<tCustomMessage>> adaptor(std::shared_ptr<BaseSyncJoin<tOut>> ptr){
        /*
         * The BaseNode functionality does not depend on the input message type that has to
         * be derived from the BaseMessage. So it is completely safe to transfer smart pointers
         * using static cast through the void pointer. It is a safe UPCAST for the message type.
         */
        auto void_ptr = std::static_pointer_cast<void>(ptr);
        auto out_ptr = std::static_pointer_cast<BaseNode<tCustomMessage>>(std::move(void_ptr));

        if(!out_ptr) throw std::runtime_error("BaseSyncJoin::adaptor cast failed");
        return out_ptr;
    }

protected:

    /*
     * The message synchronization is based on two rules:
     * 1. UID of the messages does not change between the Splitter
     * and SyncJoin nodes;
     * 2. All channels of all input pipes are FIFO. In the opposite case
     * the data loss will rise significantly.
     */

    virtual bool process_usr_msg(tPtrIn&& msg_in){
        if(!next){
            std::cerr << name << " broken pipe" << std::endl;
            return false;
        }

        if(src_list.empty()){
            std::cerr << name << " no sources registered" << std::endl;
            return false;
        }

        if(!func_join){
            std::cerr << name << " no join function that knows how to create tPtrOut" << std::endl;
            return false;
        }

        if(std::find(src_list.begin(), src_list.end(), msg_in->sent_from) != src_list.end()) {

            auto msg_uid = msg_in->get_uid();

            // collect messages
            msg_sets[msg_uid].insert(msg_in->sent_from);
            queues[msg_in->sent_from].push_back(msg_in);

            // check joinable condition
            if (msg_sets[msg_uid].size() == src_list.size()) {

                // create new message block to be sent to the next node
                auto msg_block = tPtrMsgBlock(new typename tPtrMsgBlock::element_type());

                // remove old messages from all queues the msg_sets and fill MsgBlock
                for(auto& kv_queues : queues){

                    auto& msg_queue = kv_queues.second;

                    // all messages that arrived before joining point can be removed (FIFO property)
                    // both from the msg_sets...
                    MsgDeque::iterator old_it = msg_queue.begin();
                    while((*old_it)->get_uid() != msg_uid){
                        msg_sets.erase((*old_it)->get_uid());
                        old_it++;
                    }

                    // add message to msg_block
                    (*msg_block)[kv_queues.first] = *old_it;

                    // ... and removed from the queues
                    msg_queue.erase(msg_queue.begin(), old_it + 1);
                }

                // erase joined message from msg_sets
                msg_sets.erase(msg_uid);

                // store attached data from the message that arrived from the first pipe
                // added to src_list
                BaseMessage tmp(*msg_block->at(*src_list.begin()));

                // call joining function, only user knows how to join data from different pipes
                auto out_msg = func_join(move(msg_block));
                if(!out_msg) return true;

                // atomatically manage attached data to be sure that at least
                // msg UID will be kept correct for the newly created out_msg
                if(out_msg->keep_prev_attached_data) {
                    out_msg->init_attached_data(tmp);
                }else{
                    out_msg->keep_prev_attached_data = true;
                }

                return next->put(func_join(move(msg_block)), this->uid, pol);
            }
        }

        return true;
    };

private:

    // key is the source uid, value is it's local input deque
    std::map<unsigned int, MsgDeque> queues;

    // key is the message uid, value is source uid from
    // where the message with the same uid was already received
    std::map<unsigned int, std::set<unsigned int>> msg_sets;


private:

    // sources (by uid's) that will be considered as joinable
    std::list<unsigned int> src_list;

    // function to be called each time join condition is met
    tFuncJoin func_join;

    // next node in the processing chain
    tPtrNext next;

    // the policy to be used when this node is sending
    // a new message to the "next" node, can be WAIT or DROP
    QUEUE_POLICY pol;
};

#endif //DISTPIPELINEFWK_BASE_SYNC_JOIN_H
