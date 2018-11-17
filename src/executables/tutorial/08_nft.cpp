//
// Created by morrigan on 10/16/18.
//

/*
 * Comparison of Numeric Fourier Transorm with DFT
 */
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <utility>
#include <iomanip>

#include "scope_messages/psmsgmultisig_txt.h"
#include "analytic_num_ft.h"
#include "gsl_1d_interpol.h"
#include "plotscope.h"
#include "genoscdevice.hpp"
#include "base_source.hpp"
#include "window.hpp"
#include "dft_periodic.h"
#include "ft_utils.h"

#include "gsl/gsl_errno.h"

using namespace std;

/*
 * DESCRIPTION
 *
 * This this example we compare a continuous Fourier Transform over the period of
 * the continuous periodical signal with the corresponding Discrete Fourier Transform.
 *
 * The continous signal is given with the InpSignal struct, that implements a standard
 * representation for continous signals (see dsp/standards.h). The continous Fourier
 * transform is evaluated by numerical integration using the GSL library integration
 * routine for oscillatory functions (see dsp/fourier/analytic_num_ft.h).
 *
 * Here it is changed the number of sampling points over the signal period that ranges from
 * N = 1 to 25 points. When the number of sampling points is too small to discretize the maximum
 * frecuecy present in the sugnal, the spectrum overlap is observed. Also, when the total number of
 * sampling points increase the total energy of the discrete signal is also increased.
 */

using tSource = BaseSource<PSIMsg>;
using tDevice = GenOscDevice<PSIMsg>;

// Numeric integration is divergent sometimes, this error handler produce
// errors that can be cached in a C++ manner
void gsl_err_handler (const char * reason,
              const char * file,
              int line,
              int gsl_errno){
    cerr << "gsl: " << string(file) << ":" << line << ":  " << string(reason) << "; #" << gsl_errno << endl;
    throw runtime_error("gsl_err_handler");
}

// Using an analytic continuous periodic input signal
struct InpSignal : IContinuousRR{
    InpSignal(){ interval = tInterval(n_inf, p_inf); }
    double evaluate(double t){ return sin(5*t*2*M_PI/T) + 0.5*sin(2*t*2*M_PI/T); }
    double get_period(){ return T; }
    void set_interval(tInterval ivl){}

private:
    double T = 10;
} inp_signal;


// Numerical spectrum has to be calculated just once, it is a reference data
struct NumSpectrum : public PSMsgMultiSigTxt::tSignal{
    static NumSpectrum& instance(){
        static NumSpectrum ns;
        return ns;
    }

private:
    NumSpectrum(){
        AnalyticNumFT ft(20);
        const double T = inp_signal.get_period();
        double w_min = -80.0/T;
        double w_max = 80.0/T;
        const double dw = (w_max - w_min)/300.0;

        for(double w = w_min; w <= w_max; w += dw) {
            try {
                auto mag = abs(ft.integrate(&inp_signal, 0, T, w));
                xy_data.push_back(make_pair(w, mag));
            } catch (...) {
                cout << "NumSpectrum: w = " << w << " failed" << endl;
            }
        }

        with = "with lines lt rgb '#000000' title 'Continuous FT over the period'";
    };
};

// Source function for the PlotScope device (calculate DFT and prepare plotscope message)
double N = 1;
shared_ptr<PSIMsg> src_proc(){

    auto out_msg = shared_ptr<PSMsgMultiSigTxt>(new PSMsgMultiSigTxt());

    // reference spectrum plot
    out_msg->signals.push_back(NumSpectrum::instance());

    // create discrete signal by dividing the continuous signal period onto the N intervals
    const double T = inp_signal.get_period();
    vector<double> pts(N);

    for(int i = 0; i < N; i++){
        pts[i] = inp_signal.evaluate(T*(double)i/N);
    }

    // apply discrete fourier transform (the 'pts' vector will be doubled)
    DFT dft;
    dft.initialize(N, DFT::REAL);
    dft.dft(pts, DFT::REAL);

    // move zero frecuency to the center
    ft::shift_dft_right(pts);
    auto w = ft::dft_freq_map(N, T, ft::SPECTRUM_TYPE::FT_STANDARD);

    // append the resulting discrete spectrum to the plot and add legends
    PSMsgMultiSigTxt::tSignal s1, s2;
    for(int i = 0; i < N; i++) {
        auto y = abs(complex<double>(pts[2 * i], pts[2 * i + 1]));
        s1.xy_data.push_back(make_pair(w[i], y/10.0));
        s2.xy_data.push_back(make_pair(w[i], y/10.0));
    }

    stringstream s1_ss;
    s1_ss << "with impulses lt rgb '#FF0000' title 'DFT for N = "
     << std::setw(2) << std::setfill('0') << N << "'";

    s1.with = s1_ss.str();
    s2.with = "with points pt 7 lt rgb '#FF0000' notitle";

    out_msg->signals.push_back(s1);
    out_msg->signals.push_back(s2);

    N = N > 25 ? 1 : N+1;

    return out_msg;
}

int main(int argc, char** argv){

    gsl_set_error_handler(&gsl_err_handler);

    auto src = NodeFactory::create<tSource>(src_proc, QUEUE_POLICY::WAIT);
    auto dev = NodeFactory::create<tDevice>();

    src->set_target(dev);

    //customize gnuplot output
    PlotScope* ps = new PlotScope(dev, 50);
    ps->push_gp("set xrange [-8.0 : 8.0]\n");
    ps->push_gp("set yrange [0 : 1.5]\n");
    ps->push_gp("set format y ''\n");

    //create windows and renderers (Oscilloscope)
    Window w(unique_ptr<PlotScope>(move(ps)));
    ps = nullptr;
    w.create_window();

    while(1){
        src->put(MSG_CMD::ACQUIRE, 0, QUEUE_POLICY::DROP);
        if(!w.is_running()) break;
        this_thread::sleep_for(chrono::milliseconds(400));
    }

    return 0;
}

/*
 * TEST QUESTIONS
 *
 * 1. Write analytical expression for the Fourier integral that is calculated here and add this
 * integral into the plotter window.
 *
 * 2. Explain what is the meaning of the "DFT overlap" and indicate for which N it is observed here.
 * Which theorem in not satisfied when the overlap appears?
 *
 * 3. Explain why the maximum of the low frequency peak does not exactly coincide with the DFT line.
 *
 * 4. Propose analytically a scale factor for the DFT spectra that would produce a coincidence with the
 * FT peak magnitudes in the absence of overlap. Scale the DFT spectra.
 *
 * 5. Explain on the ployt what are the coresponding Fourier Series lines.
 *
 * 6. Why InpSignal require set_interval(...) stub implementation?
 *
 * 7. Where where the NumSpectrum::with field is defined?
 */

