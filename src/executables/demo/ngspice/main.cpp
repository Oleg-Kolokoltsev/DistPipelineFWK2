//
// Created by morrigan on 4/5/19.
//

#include <iostream>

using namespace std;

#include "ngspice_src.h"

using tSource = NgSpiceSRC;
using tDevice = BaseNode<NgSpiceOutPkt>;

bool dev_proc(shared_ptr<NgSpiceOutPkt>&& msg){

    // transient example, getting chuncs of data
    for(int i = 0; i < msg->vectors.at("time").size(); i++){
        cout << "time: " << msg->vectors.at("time")[i].real()
             << ", n_2: " << msg->vectors.at("n_2")[i].real()
             << ", n_3: " << msg->vectors.at("n_3")[i].real()
             << endl;
    }

    return true;
}

int main(int argc, char** argv){

    // create and start all nodes
    auto dev =    NodeFactory::create<tDevice>(dev_proc, "Console device");
    auto src =    NodeFactory::create<tSource>("rc.cir", 10);

    // specify processing chain
    src->set_target(dev);

    // start simulation
    src->simulate();

    // await for 5 sec and than exit
    this_thread::sleep_for(chrono::milliseconds(5000));

    return 0;
}

