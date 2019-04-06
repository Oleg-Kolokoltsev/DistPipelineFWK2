//
// Created by morrigan on 4/5/19.
//

#include <iostream>

using namespace std;

#include "ngspice_src.h"

using tSource = NgSpiceSRC;
using tDevice = BaseNode<NgSpiceOutPkt>;

bool dev_proc(shared_ptr<NgSpiceOutPkt>&& msg){
    cout << msg->perc_completed << "%" << endl;
    return true;
}

int main(int argc, char** argv){

    // create and start all nodes
    auto dev =    NodeFactory::create<tDevice>(dev_proc, "Console device");
    auto src =    NodeFactory::create<tSource>("rc.cir", 10);

    // specify processing chain
    src->set_target(dev);

    // await for 5 sec and than exit
    this_thread::sleep_for(chrono::milliseconds(5000));

    return 0;
}

