//
// Created by morrigan on 8/24/18.
//

#include <random>

#include "base_source.hpp"
#include "plotscope.h"
#include "genoscdevice.hpp"
#include "window.hpp"
#include "psimsg.h"

using namespace std;

using tSource = BaseSource<PSIMsg>;
using tDevice = GenOscDevice<PSIMsg>;

std::default_random_engine generator; //random number generator
std::normal_distribution<double> distribution(0,0.02); //random number distribution

double fi = 0;
shared_ptr<PSIMsg> src_proc(){

    int N = 200;
    auto out_msg = shared_ptr<PSMsgNSigTxt>(new PSMsgNSigTxt(N, 2));
    out_msg->x_min = 0.0;
    out_msg->x_max = 2*M_PI;

    double dx = (out_msg->x_max - out_msg->x_min)/(double)N;
    double x = out_msg->x_min, y1, y2;
    fi += 0.01;

    for(int i = 0; i < N; i++) {

        y1 = sin(x) + distribution(generator);
        y2 = cos(x + fi) + distribution(generator);
        x += dx;

        gsl_matrix_set(out_msg->mat.get(), i, 0, y1);
        gsl_matrix_set(out_msg->mat.get(), i, 1, y2);

    }

    return out_msg;
}

int main(int argc, char** argv){
    auto src = NodeFactory::create<tSource>(src_proc, QUEUE_POLICY::WAIT);
    auto dev = NodeFactory::create<tDevice>();

    src->set_target(dev);

    //customize gnuplot output
    PlotScope* ps = new PlotScope(dev, 50);
    ps->push_gp("set xrange [0 : 6.2856] \n");

    //create windows and renderers (Oscilloscope)
    Window w(unique_ptr<PlotScope>(move(ps)));
    ps = nullptr;
    w.create_window();
    
    while(1){
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::DROP);
        if(!w.is_running()) break;
        this_thread::sleep_for(chrono::milliseconds(40));
    }

    return 0;
}