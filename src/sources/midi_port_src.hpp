//
// Created by morrigan on 3/2/19.
//

#ifndef DISTPIPELINEFWK_MIDI_PORT_H
#define DISTPIPELINEFWK_MIDI_PORT_H

#include <iostream>
#include <cstdlib>
#include <memory>
#include <exception>

#include <RtMidi.h>

#include "base_source.hpp"

struct MidiOutPkt : public BaseMessage{
    // empty constructor
    MidiOutPkt(){}

    // copy constructor
    MidiOutPkt(const MidiOutPkt& pkt) : BaseMessage(pkt), bytes{pkt.bytes} {}

    // Each MIDI message contains a sequence of bytes. This sequence can have a
    // different device length, and the data mapping is device dependent. Normally
    // this generic MidiOutPkt shell be sent to the filter that would convert it into the
    // commands to other application blocks.

    // Also, you can inherit this class to incapsulate device dependent mappings and
    // generate meaningful output packets.
    std::vector<unsigned char> bytes;

    double deltatime;
};

class MidiPortSRC : public BaseSource<MidiOutPkt>{

protected:
    using tPtrRtMidiIn = std::shared_ptr<RtMidiIn>;

public:
    using tBase = BaseSource<MidiOutPkt>;
    using tPtrOut = typename tBase::tPtrOut;

    MidiPortSRC(int nPort = -1) :
    tBase(nullptr, QUEUE_POLICY::DROP, std::string("MidiPort #") + std::to_string(nPort)) {
        try {
            midiin = tPtrRtMidiIn(new RtMidiIn());

            // Enumerate available ports
            auto nPorts = midiin->getPortCount();
            if(nPorts == 0) throw std::runtime_error("No ports available");
            std::cout << tBase::name << ": There are " << nPorts << "input sources available.\n";

            // Print found MIDI device names
            for ( unsigned int i = 0; i < nPorts; i++ )
                std::cout << "  Input Port #" << i
                     << ": " << midiin->getPortName(i) << std::endl;

            // Set callback function.  This should be done immediately after
            // opening the port to avoid having incoming messages written to the
            // queue.
            midiin->setCallback(&MidiPortSRC::on_midi_event, this);

            // Don't ignore sysex, timing, or active sensing messages.
            midiin->ignoreTypes(false, false, false);

            // Select device
            if(nPort == -1 || nPort > nPorts-1){
                std::cout << tBase::name << ": no port was passed to constructor - select the port!\n";
                return;
            }else{
                midiin->openPort(nPort);
            }

            std::cout << tBase::name << ": Listening to the MIDI port #" << nPort << std::endl;

        }catch(RtMidiError &err){
            err.printMessage();
        }catch(std::runtime_error& err){
            std::cerr << tBase::name << " (constructor): runtime error " << err.what() << std::endl;
        }catch(...){
            std::cerr << tBase::name << " (constructor): unhandled MIDI error\n";
        }
    }

private:
    static void on_midi_event(double deltatime, std::vector< unsigned char > *message, void *thisClassObj){

        // send tOut message
        auto srcObj = static_cast<MidiPortSRC*>(thisClassObj);

        if(srcObj->next){
            auto out_msg = tPtrOut(new MidiOutPkt);

            out_msg->sent_from = srcObj->uid;
            out_msg->bytes = std::move(*message);
            out_msg->deltatime = deltatime;

            srcObj->next->put(move(out_msg), srcObj->uid, srcObj->pol);
        }

    }

protected:
    tPtrRtMidiIn midiin;
};

#endif //DISTPIPELINEFWK_MIDI_PORT_H
