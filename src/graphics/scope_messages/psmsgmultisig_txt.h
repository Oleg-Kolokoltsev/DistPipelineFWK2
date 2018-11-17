//
// Created by morrigan on 11/7/18.
//

#ifndef DISTPIPELINEFWK_PSMSGMULTISIG_H
#define DISTPIPELINEFWK_PSMSGMULTISIG_H

#include <map>
#include <vector>
#include <list>

#include "psimsg.h"

struct PSMsgMultiSigTxt : public PSIMsg{

    struct tSignal{

        // empty constructor
        tSignal(){};

        // copy constructor
        tSignal(const tSignal& s){
            this->xy_data = s.xy_data;
            this->with = s.with;
        }

        // signal points (x,y pairs)
        std::vector<std::pair<double, double>> xy_data;

        // optional parameter,
        // if not empty, this string will replace the default "with lines notitle" without any verifications,
        // so enjoy the gnuplot error mesages!
        std::string with;
    };

    // an empty constructor does nothing
    PSMsgMultiSigTxt(){}

    // copy constructor
    PSMsgMultiSigTxt(const PSMsgMultiSigTxt& msg) : PSIMsg(msg){
        this->signals = msg.signals;
    }

    // generates a gnuplot command based on the structure field values, can be called anytime and anywhere
    virtual void get_data(std::string& dest) const;

    // required
    std::list<tSignal> signals;
};

#endif //DISTPIPELINEFWK_PSMSGMULTISIG_H
