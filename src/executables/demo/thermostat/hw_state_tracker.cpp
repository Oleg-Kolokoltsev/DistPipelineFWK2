//
// Created by morrigan on 6/24/18.
//

#include <cctype>
#include <sstream>

#include "hw_state_tracker.h"

using namespace std;

HWStateTracker::HWStateTracker(): tBase(nullptr, QUEUE_POLICY::DROP, "HWStateTracker"){
    last_state.T_in = 293;
    last_state.T_out = 0; // not used
    last_state.T_targ = 293;
    last_state.E = 0;
};


HWStateTracker::tPtrOut
HWStateTracker::internal_filter(tPtrIn&& msg_in){

    auto line = process_raw(move(msg_in));
    if(line.empty()) return nullptr;

    // the line is ok, can parse
    cout << line << endl;
    char front = line.front(); line.erase(0,1);
    istringstream ss(line);
    int adc;

    switch(front){
        case 'T': ss >> adc; last_state.T_in = temperature_from_adc(adc); break;
        case 'C': ss >> adc; last_state.T_targ = target_temperature_from_adc(adc); break;
        case 'E': ss >> adc; last_state.E = adc; break;
    }

    HWStateTracker::tPtrOut msg_out(new HWStateTracker::tPtrOut::element_type);

    msg_out->T_in = last_state.T_in;
    msg_out->E = last_state.E;
    msg_out->T_targ = last_state.T_targ;
    msg_out->cmd = MSG_CMD::NONE;

    cout << "Temp: " << (msg_out->T_in - 273)
         << "; Ctrl: " << msg_out->E
         << "; Target T.: " << (msg_out->T_targ - 273) << ";" << endl;


    return msg_out;
}

double
HWStateTracker::temperature_from_adc(int adc){
    // approx linear calibration
    const double adc1 = 20.0;    const double T1 = 289.0; // [K]
    const double adc2 = 1000.0;  const double T2 = 303.0; // [K]

    // T = a*adc + b;
    double a = (T2 - T1)/(adc2 - adc1);
    double b = (T1*adc2 - T2*adc1)/(adc2 - adc1);

    return a*adc + b;
}

double
HWStateTracker::target_temperature_from_adc(int adc){
    return ((double) adc / 1024.0)*16.0 + 289.0;
}

string HWStateTracker::process_raw(tPtrIn&& msg){
    string part(msg->block.begin(), msg->block.end());
    string terminated_line;

    // detect line break and use it
    auto pos = part.find('\n');
    if(pos == string::npos){
        // line is not terminated yet
        curr_line += part; return terminated_line;
    }else{
        // line was terminated, can analyze
        curr_line.append(part, 0, pos);
        terminated_line = curr_line;
        curr_line.clear();
        curr_line.append(part, pos + 1, part.length() - pos);
    }

    // the line is broken (process start problem), so ignore and return an empty line
    if(terminated_line.size() == 0 || !isalpha(terminated_line.front())) {
        // not yet finished,  return empty line
        terminated_line.clear();
        return terminated_line;
    };

    // the line is ok, can parse
    return terminated_line;
}
