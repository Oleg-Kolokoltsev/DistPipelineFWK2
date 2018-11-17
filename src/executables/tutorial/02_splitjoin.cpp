//
// Created by morrigan on 9/27/18.
//

#include <iostream>
#include <chrono>

#include "base_source.hpp"
#include "base_filter.hpp"
#include "base_splitter.hpp"
#include "base_sync_join.hpp"

using namespace std;
using namespace std::chrono;

/*
 * The source sends a single integer value from the global counter 'i'
 * to splitter that duplicates this packet into two chains - one through
 * the filter and the other - directly to the SyncJoin node
 */
using tSource = BaseSource<GenericDataPkt<int>>;
using tSplitter = BaseSplitter<GenericDataPkt<int>>;

/*
 * The filter artificially works slower than source, so we can see how many
 * messages where eliminated between synchronization points.
 * Also filter has the other output message type, that is typical in real computations.
 */
using tFilter = BaseFilter<GenericDataPkt<int>, GenericDataPkt<double>>;

/*
 * SyncJoin does not send any messages to anywhere (note 'return nullptr' in the join_proc),
 * so the 'BaseMessage' template parameter and tDevice node are just stubs.
 */
using tSyncJoin = BaseSyncJoin<BaseMessage>;
using tDevice = BaseNode<BaseMessage>;

// human readable type aliases
using tPtrSrcMsg = tSource::tPtrOut;
using tSrcMsg = tPtrSrcMsg::element_type;
using tPtrFilterMsg = tFilter::tPtrOut;
using tFilterMsg = tPtrFilterMsg::element_type;
using tPtrJoinedMsg = tSyncJoin::tPtrOut;
using tJoinedMsg = tPtrJoinedMsg::element_type ;

/*
 * Random number generator that is used to simulate irregular
 * computation time in the filter (for example an iterative process).
 */
default_random_engine gen;
uniform_int_distribution<int> dist(5,15);

int i = 0;
tPtrSrcMsg src_proc(){
    auto out_msg = tPtrSrcMsg(new tPtrSrcMsg::element_type);
    out_msg->val = i++;
    return out_msg;
}

tPtrFilterMsg filter_proc(tPtrSrcMsg&& in_msg){
    auto out_msg = tPtrFilterMsg(new tFilterMsg);

    /*
     * The filter function is much slower than source (see main function).
     * This way we can simulate different situations that can happen in
     * the real world computations, that can be seen from the console output.
     */
    this_thread::sleep_for(milliseconds(dist(gen)));

    // filtered data is always 10 times smaller than not filtered
    out_msg->val = (double)in_msg->val/10.0;
    return out_msg;
}

/*
 * The function where we join the data from different channels need to make a
 * message data type DOWNCAST (from the BaseMessage) for each channel separately.
 * Than it is possible to fill the output message data.
 *
 * Also this function is responsible for the hidden data that will be attached to the
 * output message. In this program such data is not used, however, the mechanism is shown
 * clearly (see the comments below).
 */
tPtrJoinedMsg join_proc(tSyncJoin::tPtrMsgBlock&& in_msg_block){

    // dummy output message (will not be sent to anywhere)
    auto out_msg = tPtrJoinedMsg(new tJoinedMsg);

    cout << "JOIN POINT" << endl;

    for(auto& kv : *in_msg_block){

        auto src_name = NodeFactory::node_name(kv.first);

        if(src_name.compare("filter_chain") == 0){

            auto filtered_msg = dynamic_pointer_cast<tFilterMsg>(kv.second);
            if(!filtered_msg) throw runtime_error("dynamic cast failed for filter_chain");
            cout << "id" << filtered_msg->get_uid() << " filter: " << filtered_msg->val << endl;

            // take attached data from the filtered pipe
            out_msg->init_attached_data(*filtered_msg);
            out_msg->keep_prev_attached_data = false;

        }else if(src_name.compare("splitter_chain") == 0){

            auto src_msg = dynamic_pointer_cast<tSrcMsg>(kv.second);
            if(!src_msg) throw runtime_error("dynamic cast failed for splitter_chain");
            cout << "id" << src_msg->get_uid() << " splitter: " << src_msg->val << endl;

        }else{
            cerr << "WARNING: not all messages from in_msg_block was recognized (shell never happen)" << endl;
        }

    }

    cout << endl;

    return nullptr;
}

// device stub (endpoint that is required by BaseSyncJoin)
bool dev_proc(tPtrJoinedMsg&& in_msg){
    return true;
}

int main(int argc, char** argv){

    auto src =    NodeFactory::create<tSource>(src_proc);
    auto filter = NodeFactory::create<tFilter>(filter_proc, QUEUE_POLICY::DROP, "filter_chain");
    auto split =  NodeFactory::create<tSplitter>(QUEUE_POLICY::DROP, "splitter_chain");
    auto join =   NodeFactory::create<tSyncJoin>(join_proc);
    auto dev =    NodeFactory::create<tDevice>(dev_proc);

    join->reg_source_uid(split->get_uid());
    join->reg_source_uid(filter->get_uid());

    src->set_target(split);
    split->add_target(filter);
    split->add_target(tSyncJoin::adaptor<tSrcMsg>(join));
    filter->set_target(tSyncJoin::adaptor<tFilterMsg>(join));
    join->set_target(dev);

    for(int i = 0; i < 1000; i++) {
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::WAIT);
        this_thread::sleep_for(chrono::microseconds(20));
    }

    //this_thread::sleep_for(chrono::milliseconds(100));

    return 0;
}

/*
 * TEST QUESTIONS
 *
 * 1. Draw the circuit, specify message types in the channels and their directions.
 *
 * 2. Explain why in the beginning the join point appear on each global counter value, however after
 * some messages the counter "distance" between join points significantly increase. How to estimate
 * the distance between counters of the neighbour join points?
 *
 * 3. What will happen if we change splitter policy to WAIT?
 *
 * 4. How to get nearly 1000 join points by only using the QUEUE_POLICY in different nodes?
 *
 * 5. Why in the join_proc is used dynamic_pointer_cast and in the tSyncJoin::adaptor is used a
 * static_pointer_cast? Why the pointer_cast was used? - It was possible to create pointers from scratch!
 *
 * 6. Why it can be important to track attached data manually? What happen if we do not manage
 * it explicitly in the join_proc?
 */

