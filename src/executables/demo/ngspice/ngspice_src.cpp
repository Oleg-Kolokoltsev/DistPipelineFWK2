//
// Created by morrigan on 4/5/19.
//

#include <regex>

#include <boost/algorithm/string.hpp>

#include "ngspice_src.h"

using namespace std;

NgSpiceSRC::NgSpiceSRC(string cir_file, int slice_data):
    cir_file{cir_file},
    slice_data{slice_data},
    is_loaded{false},
    is_halted{false},
    out_msg{tPtrOut(new tPtrOut::element_type)},
    tBase(nullptr, QUEUE_POLICY::DROP, string("NgSpiceSRC #") + cir_file){

    // In the case if a circuit file was specified inside constructor, thread of
    // the current NgSpiceSRC will load that file. In the opposite case a circuit can be
    // loaded in the message loop via user messages.
    // todo: bg_run has to be executed from the main thread, not here!!!
    if(!cir_file.empty()){
        ngSpice_Init(getchar_cb, getstat_cb, exit_cb, data_cb, initdata_cb, simulation_runs_cb, this);
        ngSpice_Command((char*) cir_file.c_str());
        //ngSpice_Command((char*) "bg_run");
        is_loaded = true;
    }
}

NgSpiceSRC::~NgSpiceSRC(){
    if(ngSpice_running())
        ngSpice_Command((char*) "bg_halt");

    // todo: use mutex here!!!
    ngSpice_Command((char*) "quit");
}

void
NgSpiceSRC::simulate(){
    ngSpice_Command((char*) "bg_run");
};

// ************ NgSpice CALLBACKS ***********************************

int
NgSpiceSRC::getchar_cb(char* outputreturn, int ident, void* userdata){

    auto srcObj = static_cast<NgSpiceSRC*>(userdata);

    string msg(outputreturn);
    if(!msg.find("stderr")){
        cerr << regex_replace(msg, regex("stderr"), "") << endl;
    }else{
        cout << regex_replace(msg, regex("stdout"), "") << endl;
    }
    return 0;
}

int
NgSpiceSRC::getstat_cb(char* outputreturn, int ident, void* userdata){

    auto srcObj = static_cast<NgSpiceSRC*>(userdata);

    // all ASCII whitespace characters
    string delimiters("\x20\x0c\x0a\x0d\x09\x0b");

    // split string onto the words
    vector<string> words;
    string s(outputreturn);
    boost::split(words, s, boost::is_any_of(delimiters));

    // find a word that has a percent sign if any - update the output packet current
    // completeness percentage
    for(const auto& w : words){
        auto perc_it = w.find('%');
        if (perc_it != std::string::npos && perc_it + 1 == w.length()) {
            try {
                srcObj->local_state_mtx.lock();
                srcObj->out_msg->perc_completed = stof(w.substr(0, perc_it));
                srcObj->local_state_mtx.unlock();
            }catch(...){}
        }
    }

    return 0;
}

int
NgSpiceSRC::data_cb(pvecvaluesall vdata, int numvecs, int ident, void* userdata){

    auto srcObj = static_cast<NgSpiceSRC*>(userdata);

    // TODO: remove this check if it never happens
    if(vdata->veccount == 0) {
        cerr << "WARNING: No vectors got from NgSpice" << endl;
        return 0;
    }

    // Add new point to the output vector
    srcObj->comp_state_mtx.lock();
    for(int i = 0; i < vdata->veccount; i++)
        srcObj->out_msg->vectors[string(vdata->vecsa[i]->name)].push_back(
                complex<double>(vdata->vecsa[i]->creal, vdata->vecsa[i]->cimag)
        );
    srcObj->comp_state_mtx.unlock();

    if(vdata->veccount != srcObj->out_msg->vectors.size())
        cerr << "WARNING: Got too few vectors from NgSpice, unexpected behavior" << endl;

    if(srcObj->slice_data > 0 &&
       srcObj->out_msg->vectors.begin()->second.size() >= srcObj->slice_data)
        srcObj->send_out_message();

    return 0;
}

int
NgSpiceSRC::initdata_cb(pvecinfoall intdata, int ident, void* userdata){

    auto srcObj = static_cast<NgSpiceSRC*>(userdata);

    srcObj->local_state_mtx.lock();

    srcObj->init_data.name = string(intdata->name);
    srcObj->init_data.title = string(intdata->title);
    srcObj->init_data.date = string(intdata->date);
    srcObj->init_data.type = string(intdata->type);

    for(int i = 0; i < intdata->veccount; i++)
        srcObj->init_data.vectors.push_back(string(intdata->vecs[i]->vecname));

    srcObj->local_state_mtx.unlock();

    return 0;
}

int
NgSpiceSRC::simulation_runs_cb(bool not_running, int ident, void* userdata){

    auto srcObj = static_cast<NgSpiceSRC*>(userdata);

    // computation was stopped or halted
    if (not_running){
        // if the computation was stopped, send the output packet
        if(!srcObj->is_halted)
            srcObj->send_out_message();
    }else{
        srcObj->is_halted = false;
        srcObj->is_loaded = true;
    }

    return 0;
}

int
NgSpiceSRC::exit_cb(int exitstatus, bool immediate, bool quitexit, int ident, void* userdata){
    auto srcObj = static_cast<NgSpiceSRC*>(userdata);
    srcObj->is_loaded = false;
    return 0;
}


// ************ PRIVATE HELPERS *************************************

void
NgSpiceSRC::send_out_message(){
    local_state_mtx.lock();
    if(next) {
        next->put(move(out_msg), uid, pol);
        out_msg = tPtrOut(new tPtrOut::element_type);
    }else{
        cerr << "WARNING: " << name << " broken pipe detected" << endl;
    }
    local_state_mtx.unlock();
}