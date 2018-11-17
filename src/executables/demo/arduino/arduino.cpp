//
// Created by morrigan on 2/7/18.
//

#include <iostream>
#include <vector>
#include <chrono>

#include "serial_port_src.hpp"

using namespace std;

using tSource = SerialPortSRC<BaseMessage, 1024>;
using tDevice = BaseNode<SerialOutPkt>;
using tTime = std::chrono::time_point<std::chrono::steady_clock>;

tTime prev_t = std::chrono::steady_clock::now();
double bytes_cnt = 0;
vector<double> stat_time;

template<typename TimeT = std::chrono::milliseconds>
double dt(tTime t0, tTime t1){
    auto duration = std::chrono::duration_cast<TimeT>
            (t1 - t0);
    return duration.count();
}

bool sec_passed(){
    auto diff = dt(prev_t, std::chrono::steady_clock::now());
    return diff >= 1000;
}

bool dev_proc(shared_ptr<SerialOutPkt>&& msg){
    uint16_t x;
    memcpy(&x, msg->block.data(), 2);
    //cout << x << endl;

    bytes_cnt += msg->block.size();
    if(sec_passed()) {
        cout << "bytes received for 1 sec: " << bytes_cnt << endl;
        bytes_cnt = 0;
        prev_t = std::chrono::steady_clock::now();
    }

    return true;
}

int main(){

    auto src =    NodeFactory::create<tSource>("/dev/ttyACM0");
    auto dev =    NodeFactory::create<tDevice>(dev_proc, "ConsoleDevice");

    src->set_target(dev);


    this_thread::sleep_for(chrono::seconds(200000));


    return 0;
}

