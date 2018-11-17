//
// Created by morrigan on 9/26/18.
//

//TODO: NOT WRITTEN YET, IT'S A PLOTSCOPE COPY

#include <random>
#include <complex>

#include "base_source.hpp"
#include "plotscope.h"
#include "genoscdevice.hpp"
#include "window.hpp"
#include "base_splitter.hpp"

#include "dft_filter.hpp"
#include "idft_filter.hpp"
#include "base_sync_join.hpp"

/*
 *  The analytic fourier transform satisfies the shift theorem:
 *  1. FT[s(t - t0)] = exp(-j * 2pi * w * t0) * FT[s(t)]
 *  2. S(w - w0) = FT[exp(j * 2pi * w0 * t) * s(t)]
 *
 *  Discrete Fourier transform also has the same property:
 *  1. DFT[x(n-l)] = exp(-j *2pi * k * l / N) * DFT[x(n)]
 *  2. X(k - k0) = DFT[exp(j * 2pi * n * k0 /N) * x(n)]
 *
 *  In this example it is illustrated application of the time-shift theorem for
 *  discrete case. The signal is shifted from left to right via it's spectrum
 *  multiplication and subsequent inverse transform.
 *
 *  Also here it is demonstrated that first element of the DFT spectrum is a
 *  signal average multiplied by N, so by changing this value it is possible to shift signal
 *  vertically working in a spectral band.
 *
 *  ___________________
 *  Architectural Note: both filters tDFT and tIDFT are wrappers of the same DFT class declared
 *  in dsp/fourier/discrete_ft.h. If you need to make some work in the spectral band, it can be more
 *  comfortable to create your own filter inherited from the DFT class as it is done in the
 *  dsp/quadrature_filter.hpp.
 *
 */

using namespace std;

// human readable type aliases for nodes
using tSource = BaseSource<RealSignalPkt>;
using tSplitter = BaseSplitter<RealSignalPkt>;
using tDFT = DFTFilter<RealSignalPkt, ComplexSignalPkt>;
using tShiftFilter = BaseFilter<ComplexSignalPkt, ComplexSignalPkt>;
using tIDFT = IDFTFilter<ComplexSignalPkt, RealSignalPkt>;
using tSyncJoin = BaseSyncJoin<PSIMsg>;
using tDevice = GenOscDevice<PSIMsg>;

// data packet types
using tPtrRealSignal = shared_ptr<RealSignalPkt>;
using tRealSignal = RealSignalPkt;
using tPtrComplexSignal = shared_ptr<ComplexSignalPkt>;
using tComplexSignal = ComplexSignalPkt;
using tPtrScopeSignal = shared_ptr<PSMsgNSigTxt>;
using tScopeSignal = PSMsgNSigTxt;

// number of data points
const size_t N = 512;

// create a signal to be shifted
tPtrRealSignal src_proc(){
    auto out_msg = tPtrRealSignal(new tRealSignal);
    out_msg->data.resize(N);

    // rectangular pulse
    for(int i = 0; i < N; i++)
        out_msg->data[i] = (i < N/5) ? 1.0 : 0.0;

    //for(int i = 0; i < N; i++)
    //    out_msg->data[i] = out_msg->data[i]*pow(-1.0, i);

    return out_msg;
}

// shift signal in frequency domain and shift the result vertically
size_t n0 = 0;
tPtrComplexSignal shift_filter_proc(tPtrComplexSignal&& in_msg){

    auto& c_data = in_msg->data;

    // time shift
    for(size_t i = 0; i < N; i++){
        complex<double> fi(0, -2*M_PI*i*n0/((double) N));
        complex<double> c(c_data[2*i], c_data[2*i+1]);
        c = c*exp(fi);

        c_data[2*i] = c.real();
        c_data[2*i+1] = c.imag();
    }

    // vertical shift
    c_data[0] += 0.5 * N * sin(2* M_PI * n0 / (double)N);

    // next time shift value (cycled)
    n0 = (n0 == N-1) ? 0 : n0 + 1;

    return in_msg;
}

// create PlotScope packet by joining two signals - original and transformed
tPtrScopeSignal join_proc(tSyncJoin::tPtrMsgBlock&& in_msg_block){

    auto out_msg = tPtrScopeSignal(new tScopeSignal(N, 3));
    out_msg->x_min = 0;
    out_msg->x_max = N-1;

    // add original and transformed signals to the first two columns of the plotter matrix
    for(auto& kv : *in_msg_block){

        auto src_name = NodeFactory::node_name(kv.first);

        auto msg = dynamic_pointer_cast<tRealSignal>(kv.second);
        if(!msg) throw runtime_error("dynamic cast failed");

        int col = (src_name.compare("splitter") == 0) ? 0 : 1;
        for(size_t i = 0; i < N; i++)
            gsl_matrix_set(out_msg->mat.get(), i, col, msg->data[i]);
    }

    // the third column is the vertical shift track
    for(size_t i = 0; i < N; i++){
        double dy = 0.5 * sin(2* M_PI * i / (double)N);
        gsl_matrix_set(out_msg->mat.get(), i, 2, dy);
    }
    out_msg->with[2] = string("with lines lc rgb 'black' notitle");

    return out_msg;
}

int main(int argc, char** argv){

    // create node objects and run their processing threads
    auto src = NodeFactory::create<tSource>(src_proc);
    auto splitter = NodeFactory::create<tSplitter>(QUEUE_POLICY::DROP, "splitter");
    auto dft = NodeFactory::create<tDFT>();
    auto shift_filter = NodeFactory::create<tShiftFilter>(shift_filter_proc);
    auto idft = NodeFactory::create<tIDFT>();
    auto join = NodeFactory::create<tSyncJoin>(join_proc);
    auto dev = NodeFactory::create<tDevice>();

    // register two input channels to join (packets from unregistered source nodes will be ignored)
    join->reg_source_uid(splitter->get_uid());
    join->reg_source_uid(idft->get_uid());

    // define processing graph
    src->set_target(splitter);
    splitter->add_target(tSyncJoin::adaptor<tRealSignal>(join));
    splitter->add_target(dft);
    dft->set_target(shift_filter);
    shift_filter->set_target(idft);
    idft->set_target(tSyncJoin::adaptor<tRealSignal>(join));
    join->set_target(dev);

    // customize gnuplot output
    PlotScope* ps = new PlotScope(dev, 40);

    ps->push_gp("set title 'The DFT shift theorem' \n");
    stringstream ss; ss << "set xrange [0 : " << N << "] \n";
    ps->push_gp(ss.str());
    ps->push_gp("set yrange [-1 : 1.5] \n");

    // create window and renderer (PlotScope)
    Window w(unique_ptr<PlotScope>(move(ps)));
    ps = nullptr;
    w.create_window();

    // main loop
    while(1){
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::DROP);
        if(!w.is_running()) break;
        this_thread::sleep_for(chrono::milliseconds(40));
    }

    return 0;
}