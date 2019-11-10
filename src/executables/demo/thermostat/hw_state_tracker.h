//
// Created by morrigan on 6/24/18.
//

#ifndef DISTPIPELINEFWK_HW_STATE_TRACKER_H
#define DISTPIPELINEFWK_HW_STATE_TRACKER_H

#include <string>

#include "base_filter.hpp"
#include "src_serial.hpp"
#include "messages.h"

/*
 * This decoder tracks all raw messages from Arduino, and updates it's current state.
 * The T_out has no matter, is comes from theoretical model and leaved for consistency.
 * The measured temperature is manually calibrated here, Arduino can measure voltages only.
 *
 * All that Arduino can send has the next format (xxx - is an ASCII decimal digit):
 * Txxx\n -> ~ voltage from thermal resistance, say current temperature; T_in
 * Exxx\n -> PWM duty cycle confirmation (0-127 are negative voltages, 128 - 255 are positive voltages; max. duty cycle is 127); E
 * Gxxx\n -> two bytes for target temperature setup (from 0 to 1024 <-> 16C to 30C); T_targ
 *
 */

class HWStateTracker : public BaseFilter<SerialOutPkt, PeltierStateMsg>{
public:
    using tBase = BaseFilter<SerialOutPkt, PeltierStateMsg>;
    using tPtrIn = tBase::tPtrIn;
    using tPtrOut = tBase::tPtrOut;

protected:
    friend class NodeFactory;
    HWStateTracker();

    virtual tPtrOut internal_filter(tPtrIn&&);

private:
    std::string process_raw(tPtrIn&&);

    double temperature_from_adc(int);
    double target_temperature_from_adc(int);

private:
    // an object of this class always remember the most recent state of the
    // hardware system and update it on each message received,
    // it is like a watchdog

    // the last, or say - the currently known state of the system
    PeltierState last_state;

    // raw parser temporary string, it is required to convert raw data to meaningful
    // lines that can be parsed afterwards
    std::string curr_line;
};

#endif //DISTPIPELINEFWK_HW_STATE_TRACKER_H
