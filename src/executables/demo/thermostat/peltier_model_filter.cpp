//
// Created by morrigan on 6/19/18.
//

#include "peltier_model_filter.h"

using namespace std;

void
PeltierModelFilter::main_loop(){

    int target_temp_cnt = 0;
    double currTargetTemp = 288.15; // [K]

    tPtrOut p_msg;

    while(1){
        tPtrIn curr_in = pull_msg(false);
        if(curr_in){
            if(curr_in->cmd == MSG_CMD::STOP){
                break;
            }else{
                E_curr = curr_in->val;
            }

        }

        //auto timestamp = seconds(time(NULL));

        // set current target temperature (just oscillating in a simulation)
        target_temp_cnt++;
        if(target_temp_cnt > 300) {
            target_temp_cnt = 0;
            currTargetTemp = currTargetTemp == 288.15 ? 303.15 : 288.15; // [K]
        }

        p_msg.reset(new tPtrOut::element_type);

        p_msg->T_targ = currTargetTemp;
        p_msg->T_in   = Tin_prev;
        p_msg->T_out  = Tout_prev;
        p_msg->E      = E_curr;

        //send data to the next processing node
        if(next){
            next->put(move(p_msg), this->uid, pol);
        }else{
            cerr << name << " no destination, data lost" << endl;
            p_msg.reset();
        }

        this_thread::sleep_for(chrono::milliseconds(10));

        //todo: it is possible to go back to real time or make any boost here!
        //each 10ms is 100ms -> x10 speed boost in comparison with real time
        evaluate(20);
    }
}
