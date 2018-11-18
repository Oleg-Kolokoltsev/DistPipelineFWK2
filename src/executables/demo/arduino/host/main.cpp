//
// Created by morrigan on 2/7/18.
//

#include <iostream>
#include <vector>
#include <deque>

#include "serial_port_src.hpp"
#include "base_filter.hpp"
#include "simplescope.h"
#include "window.hpp"

using namespace std;


using tSource = SerialPortSRC<BaseMessage, 1024>;


class Receiver : public BaseFilter<SerialOutPkt, RealSignalPkt>{
    using tBase = BaseFilter<SerialOutPkt, RealSignalPkt>;
    using tBuf = std::vector<unsigned char>;

public:
    Receiver() : tBase(nullptr, QUEUE_POLICY::DROP, "Receiver"){};

protected:
    virtual tPtrOut internal_filter(tPtrIn&& msg){

        const auto& blk = msg->block;

        part_data.insert(part_data.end(), blk.begin(), blk.begin() + blk.size());

        // find first occurrance of the header/footer delimiters
        auto head_it = search(part_data.begin(), part_data.end(), DATA_HEAD.begin(), DATA_HEAD.end());
        auto foot_it = search(part_data.begin(), part_data.end(), DATA_FOOT.begin(), DATA_FOOT.end());

        // wait until both header and footer are present
        if(head_it == part_data.end() || foot_it == part_data.end()) {
            return nullptr;
        }else if(part_data.size() > 1e6){
            throw runtime_error("1 Mb of data received without delimiters, check the board code");
        }

        // footer thet comes before header represent truncated old data
        if(distance(head_it + DATA_HEAD.size(), foot_it) < 0) {
            part_data.erase(part_data.begin(), foot_it + DATA_FOOT.size());
            return nullptr;
        }

        // get data block size and make some checks
        auto bytes = distance(head_it + DATA_HEAD.size(), foot_it);
        cout << "got " << bytes << " data bytes" << endl;

        if(bytes % 2 != 0)
            throw runtime_error("the data block expected is int16_t array, "
                                "it can't have odd number of bytes");
        if(bytes == 0)
            return nullptr;

        // get the data
        tBuf plain_data;
        vector<int16_t> data(bytes/2);
        plain_data.insert(plain_data.begin(), head_it + DATA_HEAD.size(), foot_it);
        memcpy(data.data(), plain_data.data(), plain_data.size());

        // erase already received block from the input queue
        part_data.erase(part_data.begin(), foot_it + DATA_FOOT.size());

        // create the output message
        tBase::tPtrOut p_msg(new tBase::tPtrOut::element_type);
        p_msg->data.resize(bytes/2);

        // int16_t -> double
        for(int i = 0; i < data.size(); i++)
            p_msg->data[i] = data[i];


        return p_msg;
    }

private:
    const tBuf DATA_HEAD = {'B','E','G','I','N'};
    const tBuf DATA_FOOT = {'E','N','D'};
    std::deque<tBuf::value_type> part_data;
};


int main(){

    auto osc = NodeFactory::create<GenOscDevice<RealSignalPkt>>();
    auto rcv = NodeFactory::create<Receiver>();
    auto src = NodeFactory::create<tSource>("/dev/ttyACM0");

    src->set_target(rcv);
    rcv->set_target(osc);

    //create windows and renderers (Oscilloscope)
    Window w(unique_ptr<SimpleScope>(new SimpleScope(osc, 50)));
    w.create_window();

    while(1){
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::DROP);
        if(!w.is_running()) break;
        this_thread::sleep_for(chrono::milliseconds(40));
    }

    return 0;
}

