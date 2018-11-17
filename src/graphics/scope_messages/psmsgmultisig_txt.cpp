//
// Created by morrigan on 11/7/18.
//

#include <sstream>

#include "psmsgmultisig_txt.h"

using namespace std;

void
PSMsgMultiSigTxt::get_data(std::string& dest) const{
    dest.clear();

    stringstream ss;
    ss << "plot \\" << endl;
    for(auto sig_it = signals.begin(); sig_it != signals.end(); ++sig_it) {
        const auto& sig = *sig_it;
        ss << " '-' ";
        if(sig.with.empty()){
            ss << "with lines notitle";
        }else{
            ss << sig.with;
        }
        ss << (distance(sig_it, signals.end()) == 1 ? "\n" : ",\\\n");
    }

    for(const auto& sig : signals){
        for(const auto& xy : sig.xy_data){
            ss << xy.first << " ";
            ss << xy.second << "\n";
        }
        ss << "e\n";
    }

    ss << "\n";

    dest = ss.str();
};

