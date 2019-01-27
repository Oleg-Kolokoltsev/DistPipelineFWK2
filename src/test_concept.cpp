//
// Manual:
// https://www.music.mcgill.ca/~gary/rtmidi/
//



#include <iostream>
#include <cstdlib>
#include <memory>

#include <RtMidi.h>

//TODO: Set RtMidi error callbacks

using namespace std;

void midi_event( double deltatime, std::vector< unsigned char > *message, void *userData )
{
    unsigned int nBytes = message->size();

    for ( unsigned int i = 0; i < nBytes; i++ )
        cout << "Byte " << i << " = " << (int)message->at(i) << ", ";
    if ( nBytes > 0 )
        cout << "stamp = " << deltatime << endl;
}


int main()
{
    try {
        auto midiin = shared_ptr<RtMidiIn>(new RtMidiIn());

        // Enumerate available ports
        auto nPorts = midiin->getPortCount();
        if(nPorts == 0) throw runtime_error("No ports available");
        cout << "There are " << nPorts << "input sources available.\n";

        // Print found MIDI device names
        for ( unsigned int i = 0; i < nPorts; i++ )
            cout << "  Input Port #" << i
                 << ": " << midiin->getPortName(i) << endl;

        // Select device
        int nPort = -1;
        while(1){
            cout << "\n\nSelect available input port:\n";
            cin >> nPort;
            if(nPort >= 0 && nPort < nPorts){
                midiin->openPort(nPort);
                break;
            }
        }

        // Set the callback function.  This should be done immediately after
        // opening the port to avoid having incoming messages written to the
        // queue.
        midiin->setCallback(&midi_event);

        // Don't ignore sysex, timing, or active sensing messages.
        midiin->ignoreTypes(false, false, false);

        // Wait until exit
        cin.get();
        cout << '\n' << "Listening to the MIDI port #" << nPort << endl;
        cout << "Press Enter to exit...\n\n";
        cin.get();

    }catch(RtMidiError &err){
        err.printMessage();
        return 1;
    }catch(runtime_error& err){
        cerr << err.what() << endl;
        return 1;
    }catch(...){
        cerr << "Unhandled MIDI error\n";
        return 1;
    }

    return 0;
}