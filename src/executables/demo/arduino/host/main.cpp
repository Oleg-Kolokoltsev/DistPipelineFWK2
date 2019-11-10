//
// Created by morrigan on 2/7/18.
//

#include <iostream>
#include <vector>
#include <deque>
#include <cstring>

#include "src_serial.hpp"
#include "base_filter.hpp"
#include "simplescope.h"
#include "window.hpp"

using namespace std;


using tSource = SerialPortSRC<BaseMessage, 1024>;

/*
 * This class receives the data sent from ../board/main.cpp
 *
 * The data format is:
 * ...BEGIN[...,uint16_t,...]END...
 *
 */
class Receiver : public BaseFilter<SerialOutPkt, RealSignalPkt>{
    using tBase = BaseFilter<SerialOutPkt, RealSignalPkt>;
    using tBuf = std::vector<unsigned char>;

public:
    Receiver() : tBase(nullptr, QUEUE_POLICY::DROP, "Receiver"){};

protected:
    virtual tPtrOut internal_filter(tPtrIn&& msg){

        const auto& blk = msg->block;

        //cout.write(blk.data(), blk.size());
        //cout << ">" << blk.size() << blk.data() << endl;

        //return nullptr;

        byte_queue.insert(byte_queue.end(), blk.begin(), blk.begin() + blk.size());

        // try to find header/footer delimiters
        auto head_it = search(byte_queue.begin(), byte_queue.end(), DATA_HEAD.begin(), DATA_HEAD.end());
        auto foot_it = search(byte_queue.begin(), byte_queue.end(), DATA_FOOT.begin(), DATA_FOOT.end());



        // wait until both header and footer are present
        if(head_it == byte_queue.end() || foot_it == byte_queue.end()) {
            return nullptr;
        }else if(byte_queue.size() > 1e6){
            throw runtime_error("1 Mb of data received without delimiters, check the board code");
        }

        // footer that comes before header represent truncated old data
        if(distance(head_it + DATA_HEAD.size(), foot_it) < 0) {
            byte_queue.erase(byte_queue.begin(), foot_it + DATA_FOOT.size());
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

        // create the output message
        tBase::tPtrOut p_msg(new tBase::tPtrOut::element_type);
        p_msg->data.resize(bytes/2);

        // get the data
        int i = 0; char lh[2]; int16_t v;
        for(auto it = head_it + DATA_HEAD.size(); it != foot_it;){
            // char -> int16_t
            lh[0] = *it; it++;
            lh[1] = *it; it++;
            memcpy(&v, lh, 2);
            // int16_t -> double
            p_msg->data[i++] = v;
        }

        // erase already received block from the input queue
        byte_queue.erase(byte_queue.begin(), foot_it + DATA_FOOT.size());

        return p_msg;
    }

private:
    const tBuf DATA_HEAD = {'B','E','G','I','N'};
    const tBuf DATA_FOOT = {'E','N','D'};
    std::deque<tBuf::value_type> byte_queue;
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

