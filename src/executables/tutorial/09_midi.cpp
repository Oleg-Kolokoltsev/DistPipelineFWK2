//
// Created by Dr. Yevgeniy olokoltsev on 3/2/19.
//

//TODO: Add description

#include "midi_port_src.hpp"

using namespace std;

// the source produce a raw MidiOutPkt each time it decets a midi event on the selected port
using tSource = MidiPortSRC<MidiOutPkt>;

// simple device node
using tDevice = BaseNode<MidiOutPkt>;

// the device node just prints MidiOutPkt to console
bool dev_proc(shared_ptr<MidiOutPkt>&& msg){
    cout << "GOT MIDI PACKET" << endl;
    return true;
}

int main(int argc, char** argv){

    // create and start all nodes
    auto src =    NodeFactory::create<tSource>(1);
    auto dev =    NodeFactory::create<tDevice>(dev_proc, "SimpleConsoleDevice");

    // specify processing chain
    src->set_target(dev);

    cout << "Press Enter within console focus to exit...\n\n";
    cin.get();

    return 0;
}