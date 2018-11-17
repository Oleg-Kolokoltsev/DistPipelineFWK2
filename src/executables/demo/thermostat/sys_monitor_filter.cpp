//
// Created by morrigan on 6/19/18.
//

#include <iostream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unistd.h>


#include "sys_monitor_filter.h"

using namespace std;

SysMonitorFilter::SysMonitorFilter() : tBase("SysMonitorFilter"){
    if(read_calibration()) {
        interpolate();
        mode = MODES::Operate;
    }

    loop_list.resize(N);
    val = U_min;
};

bool
SysMonitorFilter::process_usr_msg(tPtrIn&& msg){

    if(PIDController){

        tPtrPIDMsg pid_msg(new tPtrPIDMsg::element_type());

        if(mode == MODES::Calibrate){

            // store the last loop_buffer_sz values of the measured temperature
            loop_buffer.push_back(msg->T_in);
            while(loop_buffer.size() > loop_buffer_sz) loop_buffer.pop_front();

            // compute mean temperature
            double T_mean = 0;
            for(auto x : loop_buffer) T_mean += x;
            T_mean = T_mean/loop_buffer.size();

            // compute dispersion
            double d = 0;
            for(auto x : loop_buffer) d += pow(x - T_mean, 2.0);
            d = d/loop_buffer.size();
            d = pow(d,0.5);

            cout << "d = " << d << endl;

            // found stationary point, save pair
            if(d < 0.0005 && loop_buffer.size() == loop_buffer_sz){
                cout << "calib: E = " << msg->E << "; T_mean = "<< (T_mean - 273) << endl;

                measured.push_back(TU{.T = T_mean, .U = msg->E});
                loop_buffer.clear();

                val += (U_max - U_min)/20.0;
                if(val > U_max) {
                    interpolate();
                    save_calibration();
                    mode = MODES::Operate;
                }
            }

            // send info
            pid_msg->val = val;
            pid_msg->calibrate = true;

        }else if(mode == MODES::Operate){

            auto U_targ = fast_extrapolate(msg->T_targ);
            auto U_in = fast_extrapolate(msg->T_in);

            pid_msg->Err = U_targ - U_in;
            pid_msg->val = U_targ;
            pid_msg->calibrate = false;
        }

        PIDController->put(move(pid_msg), this->uid, QUEUE_POLICY::WAIT);

    }

    monitor_state(move(msg));

    return true;
}

void SysMonitorFilter::monitor_state(tPtrIn&& msg){

    loop_list.push_back(PeltierState(*msg));
    while(loop_list.size() > N) loop_list.pop_front();

    if(loop_list.size() != N) return;

    int i;

    if(scope_Err){
        tPtrSignalPktMsg Terr_msg(new tPtrSignalPktMsg::element_type());

        Terr_msg->data.resize(N);
        i = 0;
        for(const auto& x : loop_list) {
            Terr_msg->data[i] = x.T_in - x.T_targ;
            i++;
        }

        scope_Err->put(move(Terr_msg), this->uid, QUEUE_POLICY::DROP);
    }

    if(scope_U){
        tPtrSignalPktMsg Tu_msg(new tPtrSignalPktMsg::element_type());

        Tu_msg->data.resize(N);
        i = 0;
        for(const auto& x : loop_list) {
            Tu_msg->data[i] = x.E;
            i++;
        }

        scope_U->put(move(Tu_msg), this->uid, QUEUE_POLICY::DROP);
    }
}


void SysMonitorFilter::interpolate(){

    std::sort(measured.begin(), measured.end(), by_T());

    auto Npts = measured.size();

    for(auto tu : measured){
        T.push_back(tu.T);
        U.push_back(tu.U);
    }

    T_min = T[0]; T_max = T[Npts - 1];


    interp = shared_ptr<gsl_interp>(gsl_interp_alloc(gsl_interp_linear,Npts),
                                         [&](gsl_interp* p_ctx) {gsl_interp_free(p_ctx);});

    gsl_interp_init(interp.get(), T.data()/*x*/, U.data() /*y*/, Npts);
    acc = shared_ptr<gsl_interp_accel>(gsl_interp_accel_alloc(), [&](gsl_interp_accel* pa){ gsl_interp_accel_free(pa);});

    cout << "gsl_interp_init done, T_min = " << T_min << ", T_max = " << T_max << endl;

}

double SysMonitorFilter::fast_extrapolate(double Tin){
    // U(T) = a*T + b;

    if(Tin <= T_max && Tin >= T_min)
       return gsl_interp_eval(interp.get(), T.data()/*x*/, U.data() /**/, Tin, acc.get());

    double a = (U.front() - U.back())/(T.front() - T.back());
    double b = (U.back()*T.front() - U.front()*T.back())/(T.front() - T.back());

    return (a*Tin + b);
}

void SysMonitorFilter::save_calibration(){
    std::ofstream file;
    file.open("calibrate.dat");

    for(int i = 0; i < T.size(); i++){
        file << std::right
             << std::setfill(' ')
             << std::setprecision(5)
             << std::setw(12)
             << std::scientific
             << T[i];

        file << std::right
             << std::setfill(' ')
             << std::setprecision(5)
             << std::setw(14)
             << std::scientific
             << U[i] << '\n';
    }

    file.close();
}

bool
SysMonitorFilter::read_calibration(){
    int res = access("calibrate.dat", R_OK);
    if(res < 0){
        cout << "file calibrate.dat not exist" << endl;
        return false;
    }

    ifstream f("calibrate.dat");

    string line;
    double col1, col2;

    while (std::getline(f, line)) {

        istringstream ss(line);
        ss >> col1 >> col2;
        cout << line << " " << col1 << " " << col2 << endl;

        measured.push_back(TU{.T = col1, .U = col2});
    }

    f.close();

    return !measured.empty();
}

// no no no, BSpline is not an extrapolation
/*void SysMonitorFilter::extrapolate() {

    // number of data points to fit
    int n = measured.size();

    //number of fit coefficients
    int ncoeffs = 12;

    // B-spline order (k = 3 is for cuadratic )
    int k = 3;

    // number of breakpoints
    int nbreak = ncoeffs + 2 - k;


    // workspaces
    using tGBW = gsl_bspline_workspace;
    auto bw = shared_ptr<tGBW>(gsl_bspline_alloc(k, nbreak), [](tGBW* p){gsl_bspline_free(p);});

    using tGMLW = gsl_multifit_linear_workspace;
    auto mw = shared_ptr<tGMLW>(gsl_multifit_linear_alloc(n, ncoeffs), [](tGMLW* p){gsl_multifit_linear_free(p);});

    auto B = shared_ptr<gsl_vector>(gsl_vector_alloc(ncoeffs), [](gsl_vector* p){gsl_vector_free(p);});

    auto y = shared_ptr<gsl_vector>(gsl_vector_alloc(n), [](gsl_vector* p){gsl_vector_free(p);});
    auto c = shared_ptr<gsl_vector>(gsl_vector_alloc(ncoeffs), [](gsl_vector* p){gsl_vector_free(p);});

    // fit matrix
    auto X = shared_ptr<gsl_matrix>(gsl_matrix_alloc(n, ncoeffs), [](gsl_matrix* p){gsl_matrix_free(p);});
    // covariance matrix
    auto cov = shared_ptr<gsl_matrix>(gsl_matrix_alloc(ncoeffs, ncoeffs), [](gsl_matrix* p){gsl_matrix_free(p);});


    // use uniform breakpoints on
    gsl_bspline_knots_uniform(T_min, T_max, bw.get());

    // construct the fit matrix X
    size_t i, j;
    for (i = 0; i < n; ++i)
    {
        // compute B_j(xi) for all j
        gsl_bspline_eval(T[i], B.get(), bw.get());

        // fill in row i of X
        for (j = 0; j < ncoeffs; ++j)
        {
            double Bj = gsl_vector_get(B.get(), j);
            gsl_matrix_set(X.get(), i, j, Bj);
        }
    }

    for(i = 0; i < n; i++) { gsl_vector_set(y.get(), i, U[i]);};


    // do the fit
    double chisq;
    gsl_multifit_linear(X.get(), y.get(), c.get(), cov.get(), &chisq, mw.get());


    cout << "extrapolate to be fineshed " << endl;

    // output extrapolator
    double xi, yi, yerr;
    for (xi = T_min - 10; xi < T_max + 10; xi += 1)
    {
        gsl_bspline_eval(xi, B.get(), bw.get());
        gsl_multifit_linear_est(B.get(), c.get(), cov.get(), &yi, &yerr);
        cout << "BSpline : T = " << xi << "; U = " << yi << endl;
    }


}*/

