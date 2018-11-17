//
// Created by morrigan on 8/24/18.
//

#ifndef DISTPIPELINEFWK_GENOSCDEVICE_H
#define DISTPIPELINEFWK_GENOSCDEVICE_H

#include <string>

#include "base_node.hpp"
#include "data_packet_types.h"

/*
 * Each Oscilloscope support it's own data message(s). However, the class GenOscDevice
 * that works as a device node is the same for all Oscilloscopes. Each time
 * the new message arrive to GenOscDevice, it will replace the old one
 * stored locally. This way we guarantee that independently of the Oscilloscope speed,
 * each time the most recent frame will be shown.
 *
 * As long as different Oscilloscopes use different messages, the message type
 * is unknown in this class. A concrete graphics knows what message type
 * it is using at compile time, therefore GenOscDevice is just a template that
 * implements pull policy.
 */

template<typename tIn>
class GenOscDevice : public BaseNode<tIn> {
    static_assert(std::is_base_of<BaseMessage, tIn>(), "The tIn shell be derived from BaseMessage class");

public:
    using tBase = BaseNode<tIn>;
    using tPtrFrame = std::shared_ptr<tIn>;

    GenOscDevice(std::string name = "OscDevice"): tBase(name), new_msg(true){};

    /*
     * This function copy the most recent last_frame data into the newly allocated
     * output frame in a thread safe manner. Direct access to the last_frame is not thread safe
     * because t's data would be accessible from any other thread while it is changing.
     */
    tPtrFrame get_frame(){
        // lock mutex until the function returns (thread safety guarantee)
        std::unique_lock<std::mutex> lck(tBase::local_state_mtx);

        // the frame is not recent after the first call to this function
        new_msg = false;

        // no move on this packet, becuse of subsequent usage diversity,
        // this class guarantees that the last frame arrived will not dissapear
        return last_msg;
    }

protected:

    //the current last_pkt is just replaced with the newly arrived
    virtual bool process_usr_msg(tPtrFrame&& in_msg){
        //lock mutex until the function returns
        std::unique_lock<std::mutex> lck(tBase::local_state_mtx);
        last_msg = in_msg;
        new_msg = true;
        return true;
    };

private:
    bool new_msg;
    tPtrFrame last_msg;
};

#endif //DISTPIPELINEFWK_GENOSCDEVICE_H
