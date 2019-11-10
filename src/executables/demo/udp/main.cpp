//
// Created by morrigan on 11/9/19.
//

#include "src_udp.hpp"

using namespace std;

using tSource = UDPSource<1024>;
using tDevice = BaseNode<UDPOutPkt>;

bool dev_proc(shared_ptr<UDPOutPkt>&& msg){
    cout << "udp pkt received, size = " << msg->block.size() << endl;
    return true;
}

int main(){

    auto src = NodeFactory::create<tSource>("192.168.1.255:1234");
    auto dev = NodeFactory::create<tDevice>(dev_proc);

    src->set_target(dev);

    this_thread::sleep_for(chrono::seconds(5));

    return 0;
}
