//
// Created by morrigan on 6/18/18.
//

#ifndef DISTPIPELINEFWK_PID_CONTROLLER_H
#define DISTPIPELINEFWK_PID_CONTROLLER_H

#include <list>
#include <memory>
#include <chrono>
#include <tuple>

#include "data_packet_types.h"
#include "base_filter.hpp"

// shell be derived from BaseMessage, and if it is not the compilation assert message appear
struct PIDMsgIn : public BaseMessage{

    using tTimePoint = std::chrono::time_point<std::chrono::steady_clock>;

    // empty constructor, required to create new messages
    PIDMsgIn(){}

    // copy constructor, required for message copying,
    // please note the call to base class constructor, it is not empty
    PIDMsgIn(const PIDMsgIn& msg) : BaseMessage(msg){
        val = msg.val;
        Err = msg.Err;
        calibrate = msg.calibrate;
    }


    // target value of the control parameter
    double val;

    // error between target value and measured value
    double Err;

    // if this is true, the PIDController will just send "val" on it's output
    bool calibrate;
};


//The output message is always a scalar
struct PIDMsgOut : public BaseMessage{

    // empty constructor, required to create new messages
    PIDMsgOut(){}

    // copy constructor, required for message copying,
    // please note the call to base class constructor, it is not empty
    PIDMsgOut(const PIDMsgOut& msg) : BaseMessage(msg){
        val = msg.val;
    }

    // the physical meaning of the PID output is defined by the
    // physical meaning [units] of it's constants and the units
    // of the Err function

    // val - is a control value that can be a voltage or something
    // else that is sent to the physical system or it's model
    double val;
};

class PIDFilter: public BaseFilter<PIDMsgIn, PIDMsgOut> {

public:
    using tBase = BaseFilter<PIDMsgIn, PIDMsgOut>;
    using tPtrOut = typename tBase::tPtrOut;
    using tPtrIn = typename tBase::tPtrIn;
    using tTimePoint = PIDMsgIn::tTimePoint;
    using tSeconds = std::chrono::seconds;

protected:
    friend class NodeFactory;

    PIDFilter(QUEUE_POLICY pol = QUEUE_POLICY::WAIT, std::string name = "PIDFilter"):
    tBase(nullptr, pol, name) {}

    virtual tPtrOut internal_filter(tPtrIn&& msg){

        using namespace std;
        using namespace chrono;

        auto e = msg->Err;

        // control parameter (not a correction factor!)
        tPtrOut out_msg(new typename tPtrOut::element_type);

        // calibration mode (PID acts as a transparent node)
        if(msg->calibrate){
            out_msg->val = msg->val;
            return out_msg;
        }

        //Product
        out_msg->val = Kp*e;


        if(!circular_buf.empty()){

            //Differentiate
            out_msg->val += Kd*(e - circular_buf.back());

            //Integrate
            while(circular_buf.size() > N ) circular_buf.pop_front();
            circular_buf.push_back(e);


            double integ = 0.0;
            for(auto it = circular_buf.begin(); true; it++){
                auto next_it = it; next_it++;
                if(next_it == circular_buf.end()) break;
                integ += 0.5*((*next_it) - (*it));
            }


            out_msg->val += Ki*integ;
        }

        out_msg->val += msg->val;

        return out_msg;
    }

private:
    // P, D, I constants and back in time period for the integral
    double Kp = 0.5, Kd = 0.0, Ki = 0.5;

    // integrate for N periods into the past
    int N = 10;

    // to have a finite integration we need to remember a couple of previous steps
    std::list<double> circular_buf;
};

#endif //DISTPIPELINEFWK_PID_CONTROLLER_H
