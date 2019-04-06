//
// Created by morrigan on 13/01/18.
//

#ifndef DISTPIPELINEFWK_BASE_NODE_HPP_H
#define DISTPIPELINEFWK_BASE_NODE_HPP_H

#include <memory>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <chrono>
#include <string>
#include <functional>

#include "data_packet_types.h"
#include "node_factory.hpp"

//***************BASE THREAD WITH INPUT MESSAGE QUEUE***********************
/*
 * There is two basic usages of this class:
 * 1. There is no need to store any additional local state
 * variables in the child process
 * and it shell have just an input queue of specified messages. This
 * can be a statistics terminal
 * device or any other simple end-point. In this case we can use a
 * second constructor and specify
 * a process_usr_msg pointer to a function.
 *
 * 2. It is possible to derive any functional type from this one
 * extending/overwriting it functions with
 * something useful: IN-OUT type (filter),  stateful end-device/source etc...
 */

template<typename tIn>
class BaseNode : public CommandNode{
    static_assert(std::is_base_of<BaseMessage,tIn>(),
                  "The tIn shell be derived from BaseMessage class");

public:
    using tPtrIn = std::shared_ptr<tIn>;

protected:
    friend class NodeFactory;
    BaseNode(std::string name = "BaseNode"): CommandNode(name) {}
    BaseNode(std::function<bool(tPtrIn&&)> func_process = nullptr, std::string name="BaseNode"):
            CommandNode(name),
            func_process{func_process} {};

public:
    ~BaseNode(){

        /* Each 'BaseNode' child is a node in some distributed network
         * if a process just disappear (even gracefully), the connectivity of the graph can be altered,
         * making it impossible to run wave algorithms, such as an explicit termination.
         * So by default we start a "stop" propagation wave over all the network and therefore
         * kill it. In special cases this destructor can be overwritten.
         * Join with a thread that called destructor
         *
         */
        stop();
    }

    std::string get_name(){return name;}

    void start(){
        own_thread = std::thread(run,this);
        // wait until the process starts
        while(!v_running)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        std::cout << name << " started" << std::endl;
    }

    void stop(){
        if(!v_running) return;

        put(MSG_CMD::STOP, uid, QUEUE_POLICY::WAIT);
        own_thread.join();

        std::cout << name << " stopped" << std::endl;
    }

    bool put(tPtrIn&& val, unsigned int sent_from, QUEUE_POLICY pol = QUEUE_POLICY::DROP){

        // no messages can be accepted until the node thread was started
        // when nodes are closing their threads this is a normal situation, so return true
        if(!v_running) return true;

        // a null pointers are used when a node that has an output does not want to send
        // anything, for example it has received and processed any command message
        if(!val) return true;

        // store the msg source inside the message
        val->sent_from = sent_from;

        // lock a local state for inter-thread communication
        std::unique_lock<std::mutex> lck(local_state_mtx);

        // input queue is full
        if(in.size() > 10){
            if(pol == QUEUE_POLICY::WAIT){
                // std::cout << "BaseNode.put(): in.size() > 10" << std::endl;
                // unlock local state and wait until the queue shortens
                // when the event arrive local state will be relocked
                event.wait(lck,[this]{return in.size() <= 10;});
            }else if(pol == QUEUE_POLICY::DROP){
                // the message was sent via rvalue, so it is dropped if not stored
                // in this case the message shared_ptr<> destructor is called
                // it is a normal behaviour for slow nodes, so we return true
                return true;
            }
        }

        // push the message, notify local thread if it was blocked and
        // confirm received by returning true
        in.push_back(move(val));
        event.notify_one();

        // 'lck' destructor unlocks 'local_state_mtx' automatically
        return true;
    }

    /*
     * It is used to send a command to this particular node.
     * This function simplifies the usage of a generic "put" routine defined
     * above.
     */
    bool put(MSG_CMD cmd, unsigned int sent_from,
             QUEUE_POLICY pol = QUEUE_POLICY::WAIT,
             std::shared_ptr<ICloneable>&& user_data = nullptr){

        auto msg = tPtrIn(new typename tPtrIn::element_type);
        msg->cmd = cmd;
        msg->user_data = user_data;

        return put(move(msg), sent_from, pol);
    }

protected:

    /*
     * This is the main loop function. It is virtual, so it can be
     * rewritten in any particular node implementation, however,
     * is the most cases, the periodic calls to pull_msg->process_usr_msg
     * shell be enough. In the case when overwritten main_loop was not
     * cached by the compiler (it sometimes happen with MacOS compiler),
     * the warning will indicate on that.
     */
    virtual void main_loop(){
        tPtrIn curr_in;
        while(1){
            curr_in = pull_msg();
            if(curr_in->cmd == MSG_CMD::STOP){
                break;
            }else if(curr_in->cmd == MSG_CMD::USER && curr_in->user_data){
                //TODO: refactor user_data name, it's basic now
                curr_in->user_data->apply(this);
            }

            if(!process_usr_msg(move(curr_in))){
                std::cerr << name << " warning: process_usr_msg failed" << std::endl;
            }
        }

        // discard all messages received after the "STOP" command
        in.clear();
    }

    virtual bool process_usr_msg(tPtrIn&& msg){
        if(!func_process){
            std::cerr << name << " warning: user function not specified" << std::endl;
            return true;
        }else{
            return func_process(move(msg));
        }
    };

    tPtrIn pull_msg(bool wait = true){
        std::unique_lock<std::mutex> lck(local_state_mtx);
        if(wait){
            event.wait(lck,[=]{return !in.empty();});
        }else if(in.empty()){
            return nullptr;
        }
        tPtrIn curr_in = in.front();
        in.pop_front();
        event.notify_one();
        return curr_in;
    }

private:
    /*
     * This static function is used to start a new thread.
     * It wraps a "main_loop", which can be overwritten.
     */
    static void run(BaseNode* f){
        f->v_running = true;
        f->main_loop();
        f->v_running = false;
    };

protected:
    // the main mutex is accessible from any child class
    // so the sincrinization never breaks
    std::mutex local_state_mtx;

    //todo: user function to be called in a simple end-point node
    std::function<bool(tPtrIn&&)> func_process;

private:
    std::thread own_thread;
    std::list<tPtrIn> in;
    std::condition_variable event;
    bool v_running = false;
};

#endif //DISTPIPELINEFWK_BASE_NODE_HPP_H
