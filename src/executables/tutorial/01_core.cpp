//
// Created by Dr. Yevgeniy Kolokoltsev on 1/31/17.
//

/*
 * The DistPipelineFWK is a small framework that is aimed to parallel
 * the subsequent stages of the information processing chain on a multi core
 * systems. Each signal block (or slice) received from it's source can
 * require a subsequent application of a number of routines that we call filters
 * here. When a new signal block arrive, the other one can be found in one
 * of different processing stages. As long as multi core CPUs permit to process
 * more than one computation at the same moment, we can use all computational
 * resources of a CPU by creating more than one parallel threads (nodes).
 *
 * In this example we demonstrate the basic functionality of DistPipelineFWK framework.
 * First we show how to define a custom data packet structure MyMsg that contains a "signal" block.
 * Several standard data packets can be found in "core/data_packet_types.h" header.
 * Here, for simplicity, the signal is presented by just one value "val". In this example
 * the same packets are used between any pair of processing nodes. However,
 * there is no limitations for the signal representation. Any packet structure can be used
 * between any pair of subsequent processing nodes.
 *
 * In this example filter just adds 100 to the initial packet data "val". Finally we
 * print each packet data in the device node, that is a final point in this simple
 * processing chain. Each node (source, filter and device) process in it's own thread,
 * so all three 'proc' functions work in parallel.
 */

#include <chrono>

#include "base_source.hpp"
#include "base_filter.hpp"

// it is permitted to use namespaces in a *.cpp files, however it
// is a not recommended practice in *.h or *.hpp files
using namespace std;

/*
 * There can be any data that is processed within a pipeline. This
 * data can change from one node to another, so it is a common practice
 * to define the data structures to be passed between any pair of nodes.
 *
 * In this simple example we work with just one custom data struct MyMsg.
 * The other common structs are defined in the data_packet_types.h file.
 * Each message that is passed between two threads shell have a copy constructor that
 * is used in a generic packet manipulations inside a framework.
 */

// shell be derived from BaseMessage, and if it is not the compilation assert message appear
struct MyMsg : public BaseMessage{
    // empty constructor, required to create new messages
    MyMsg(){}

    // copy constructor, required for message copying,
    // please note the call to base class constructor, it is not empty
    MyMsg(const MyMsg& msg) : BaseMessage(msg){
        val = msg.val;
    }

    // any meaningful data fields
    double val;
};

// the source produce a MyMsg in response to each AQUIRE message received
// (see main function)
using tSource = BaseSource<MyMsg>;

// the filter modifies one MyMsg producing another MyMsg
using tFilter = BaseFilter<MyMsg, MyMsg>;

// device node prints out the data of MyMsg
using tDevice = BaseNode<MyMsg>;

/*
 * This src_proc function will be called automatically in the main_loop of the source thread.
 * The use of global variables such as "int i = 0;" below is a not recommended practice.
 * In the case when a source needs for any context data (that can be stored with a
 * source object), there shell be created a new source class derived from the BaseSource
 * or a BaseNode (see other tutorial).
 */
int i = 0;
shared_ptr<MyMsg> src_proc(){
    auto msg = shared_ptr<MyMsg>(new MyMsg);
    msg->val = i++;

    cout << "sent val: " << msg->val << endl;
    return msg;
}

/*
 * Each time when the "filter" node thread receive a new message
 * it apply a user defined algorithm "filter_proc" that modify the message data.
 *
 * In the case when filter output packet type is not the same as it's input,
 * the new packet shell be created explicitly inside of this function.
 * This case is illustrated in the other tutorial.
 */
shared_ptr<MyMsg> filter_proc(shared_ptr<MyMsg>&& in_msg){
    in_msg->val += 100;
    cout << "filtered" << endl;
    return in_msg;
}

/*
 * The device is placed at the end of the processing chain.
 * Usually it is used to show the results. It can be a graphic device,
 * a console output (as here), etc.
 *
 * Custom deviced have no limitations on what to do with already processed signal.
 * For example this signal can be sent to the outside world, (ALSA driver, DAC).
 * It can be sent via network to another CPU. In the last case, the FIFO channel
 * is required by core/base_sync_join.hpp, so better not to use protocols like
 * UDP.
 *
 * Another useful possibility is to create a server device that would
 * send a preprocessed signal to any other signal processing software,
 * like a LabView.
 */
bool dev_proc(shared_ptr<MyMsg>&& msg){
    cout << "val = " << msg->val << endl;
    return true;
}

int main(int argc, char** argv){
    /*
     * It is required to create all nodes via NodeFactory singleton. NodeFactory assigns
     * a uniqueID (UID) to each new node, start it's process when the node
     * is created and register UID<->node_object pair. When application terminates
     * the NodeFactory destructor send MSG_CMD::STOP to all nodes and joins their
     * processes with main().
     *
     * Pointer to each node is a shared_pointer container that is an automatic
     * garbage collector.
     *
     * Node UIDs are used to track the information flow: each message is derived from
     * the BaseMessage, where the msg.sent_from variable is always set to the last
     * node that processed that message. Also this UID can be used at any place
     * to get pointer to any node or to get the node name. UID is random and it's
     * lifetime is equal to the node object lifetime.
     *
     * The NodeFactory::create is used below. It is a template object
     * factory function. This function has a variadic parameters that shell coincide
     * with one of the valid constructors of the node to be created.
     * The node type is specified in the function template argument.
     *
     * The NodeFactory::create function is the only function permitted to create
     * new nodes. Direct instantiation of any node is forbidden by a protected
     * constructors and it is impossible to create nodes without NodeFactory.
     */

    // create and start all nodes
    auto src =    NodeFactory::create<tSource>(src_proc, QUEUE_POLICY::WAIT);
    auto filter = NodeFactory::create<tFilter>(filter_proc, QUEUE_POLICY::WAIT);
    auto dev =    NodeFactory::create<tDevice>(dev_proc, "DeviceNode");

    // specify processing chain
    src->set_target(filter);
    filter->set_target(dev);

    //send some AQUIRE messages to the source, each of these messages will produce a single
    //call of src_proc
    for(int i = 0; i < 10; i++)
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::WAIT);


    /*
     * In this example nodes are killed in a random order exactly after the last AQUIRE
     * message was sent to "src" node. So, in any program run, the input queue
     * of "src" will be full of AQUIRE messages. The last message to "src" node is a STOP
     * message, produced by NodeFactory destructor when main() returns. This destructor
     * also sends STOP command to all the other node threads and joins them with the main
     * function thread.
     *
     * What we would see in the program output depends on the STOP command
     * position in the "filter" and "dev" input queues. For example, if a "STOP"
     * command is sent to all before the "src" starts to invoke it's src_proc function,
     * the "filter" and "dev" threads will simply ignore all messages sent by "src" and no
     * other output will be produced. The other limiting case is when a STOP command
     * arrive to "dev" after it's queue is filled by filtered messages. In this
     * case we will see the result of 10 dev_proc invocations. All other intermediate cases
     * lie in between these two and can be observed in a series of this program runs.
     *
     * To observe the limiting case when all queues was finished it's job just in time
     * you can uncomment the next line.
     */
    //this_thread::sleep_for(chrono::milliseconds(100));

    return 0;
}