//
// Created by morrigan on 10/14/18.
//

#include <exception>
#include <iostream>

#include "gsl_1d_interpol.h"

using namespace std;

// regular constructors
GSL1DInterpolRR::GSL1DInterpolRR(const gsl_interp_type* type) : tBase(type) {};
GSL1DInterpolCR::GSL1DInterpolCR(const gsl_interp_type* type) : tBase(type) {};




// copy constructors
GSL1DInterpolRR::GSL1DInterpolRR(const GSL1DInterpolRR& obj) {
    this->type = obj.type;
    this->x_vec = obj.x_vec;
    this->y_vec = obj.y_vec;
    this->interval = obj.interval;
    this->T = obj.T;
    this->ws.reset();
    this->accel.reset();
    if (!x_vec.empty() && !y_vec.empty()) {
        update_ws(this->ws, this->y_vec);
    }else{
        clear();
    }
};

GSL1DInterpolCR::GSL1DInterpolCR(const GSL1DInterpolCR& obj){
    this->type = obj.type;
    this->x_vec = obj.x_vec;
    this->re_y_vec = obj.re_y_vec;
    this->im_y_vec = obj.im_y_vec;
    this->interval = obj.interval;
    this->T = obj.T;
    this->re_ws.reset();
    this->im_ws.reset();
    this->accel.reset();
    if(!x_vec.empty() && !re_y_vec.empty() && !im_y_vec.empty()) {
        update_ws(this->re_ws, this->re_y_vec);
        update_ws(this->im_ws, this->im_y_vec);
    }else{
        clear();
    }
}





void
GSL1DInterpolRR::set_data(const tVec &y, const tVec &x) {

    if(y.size() < gsl_interp_type_min_size(type))
        throw runtime_error("GSL1DInterpolRR::set_data: insufficient y-data points");

    if(!x.empty() && x.size() != y.size())
        throw runtime_error("GSL1DInterpolRR::set_data: inconsistent sizes of y and x data");

    // new non linear x-axis defined
    if(!x.empty()){
        x_vec = x;
        T = -1;
        ws.reset();
    // x is not defined, and y.size is inconsistent with old x size
    }else if(y.size() != x_vec.size()){
        x_autofill(y.size());
        T = -1;
        ws.reset();
    }

    y_vec = y;

    interval.set(x_vec.front(), x_vec.back());
    update_ws(ws, y_vec);
};

void
GSL1DInterpolCR::set_data(const tVec &y, const tVec &x){
    size_t N = y.size();
    if(N % 2 != 0){
        throw runtime_error("GSL1DInterpolCR::set_data: y-data is not a complex packed array");
    }else{
        N = N/2;
    }

    if(N < gsl_interp_type_min_size(type))
        throw runtime_error("GSL1DInterpolCR::set_data: insufficient y-data points");

    if(!x.empty() && x.size() != N)
        throw runtime_error("GSL1DInterpolCR::set_data: inconsistent sizes of y and x data");

    // new non linear x-axis defined
    if(!x.empty()){
        x_vec = x;
        T = -1;
        re_ws.reset();
        im_ws.reset();
        // x is not defined, and y.size is inconsistent with old x size
    }else if(y.size() != x_vec.size()){
        x_autofill(y.size());
        T = -1;
        re_ws.reset();
        im_ws.reset();
    }

    re_y_vec.resize(N);
    im_y_vec.resize(N);

    for(size_t i = 0; i < N; i++){
        re_y_vec[i] = y[2*i];
        im_y_vec[i] = y[2*i + 1];
    }

    interval.set(x_vec.front(), x_vec.back());
    update_ws(re_ws, re_y_vec);
    update_ws(im_ws, im_y_vec);
}







void
GSL1DInterpolRR::set_periodic(double dx){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolRR::set_periodic: workspace is not initialized");

    if(T > 0)
        throw runtime_error("GSL1DInterpolRR::set_periodic: periodicity is already set, make clear() first");

    // compute period
    T = x_vec.back() - x_vec.front();
    if(dx < 0) dx = T/(double)x_vec.size();
    T = T + dx;

    // close input data (see the evaluate code)
    x_vec.push_back(x_vec.back() + dx);
    y_vec.push_back(y_vec.front());

    // periodic functions are infinite by default
    interval.set(n_inf, p_inf);

    // the number of points has changed, so it is required to update the workspace
    ws.reset();
    update_ws(ws, y_vec);
}

void
GSL1DInterpolCR::set_periodic(double dx){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolCR::set_periodic: workspace is not initialized");

    if(T > 0)
        throw runtime_error("GSL1DInterpolCR::set_periodic: periodicity is already set, make clear() first");

    // compute period
    T = x_vec.back() - x_vec.front();
    if(dx < 0) dx = T/(double)x_vec.size();
    T = T + dx;

    // close input data (see the evaluate code)
    x_vec.push_back(x_vec.back() + dx);
    re_y_vec.push_back(re_y_vec.front());
    im_y_vec.push_back(im_y_vec.front());

    // periodic functions are infinite by default
    interval.set(n_inf, p_inf);

    // the number of points has changed, so it is required to update the workspace
    re_ws.reset();
    im_ws.reset();

    update_ws(re_ws, re_y_vec);
    update_ws(im_ws, im_y_vec);
}







void
GSL1DInterpolRR::set_interval(tInterval ivl){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolRR::set_interval: workspace is not initialized");

    if(T > 0){
        interval = ivl;
    }else{
        if(ivl.lower() < x_vec.front() || ivl.upper() > x_vec.back())
            throw runtime_error("GSL1DInterpolRR::set_interval: interval is "
                                "out of range of the interpolation domain of the finite function");
    }
}

void
GSL1DInterpolCR::set_interval(tInterval ivl){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolCR::set_interval: workspace is not initialized");

    if(T > 0){
        interval = ivl;
    }else{
        if(ivl.lower() < x_vec.front() || ivl.upper() > x_vec.back())
            throw runtime_error("GSL1DInterpolCR::set_interval: interval is "
                                "out of range of the interpolation domain of the finite function");
    }
}








double
GSL1DInterpolRR::evaluate(double x){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolRR::evaluate: workspace is not initialized");

    // for finite functions this is a complete interval, for periodic case it is possible to cut the interval
    // catch this exception and define the function value outside the interval
    if(!boost::numeric::in(x, interval))
        throw runtime_error("GSL1DInterpolRR::evaluate: x is out of function range");

    // periodicity permits to shift 'x' in any direction if the data is closed (y_vec[0] == y_vec[N-1])
    namespace bn = boost::numeric;
    tInterval x_int(x_vec.front(), x_vec.back());
    if(T > 0 && !bn::in(x, x_int)){
        // numerical stabilization of possible errors around boundaries
        int n1 = floor((x_vec.front() - x)/T) + 1;
        int n2 = ceil((x_vec.front() - x)/T);
        x = bn::in(x+n1*T, x_int) ? x+n1*T : x+n2*T;
    }

    double res;
    auto err = gsl_interp_eval_e(ws.get(), x_vec.data(), y_vec.data(), x, accel.get(), &res);
    alert_gsl_err(err, "GSL1DInterpolRR::evaluate:");

    return res;
};


complex<double>
GSL1DInterpolCR::evaluate(double x, ZPART z_part){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolCR::evaluate: workspace is not initialized");

    // for finite functions this is a complete interval, for periodic case it is possible to cut the interval
    // catch this exception and define the function value outside the interval
    if(!boost::numeric::in(x, interval))
        throw runtime_error("GSL1DInterpolCR::evaluate: x is out of function range");

    // periodicity permits to shift 'x' in any direction if the data is closed (y_vec[0] == y_vec[N-1])
    namespace bn = boost::numeric;
    tInterval x_int(x_vec.front(), x_vec.back());
    if(T > 0 && !bn::in(x, x_int)){
        // numerical stabilization of possible errors around boundaries
        int n1 = floor((x_vec.front() - x)/T) + 1;
        int n2 = ceil((x_vec.front() - x)/T);
        x = bn::in(x+n1*T, x_int) ? x+n1*T : x+n2*T;
    }

    complex<double> res(0.0, 0.0);
    if(z_part == CPLX || z_part == RE) {
        double re_res;
        auto err = gsl_interp_eval_e(re_ws.get(), x_vec.data(), re_y_vec.data(), x, accel.get(), &re_res);
        alert_gsl_err(err, "GSL1DInterpolCR::evaluate (real part):");
        res.real(re_res);
    }

    if(z_part == CPLX || z_part == IM) {
        double im_res;
        auto err = gsl_interp_eval_e(im_ws.get(), x_vec.data(), im_y_vec.data(), x, accel.get(), &im_res);
        alert_gsl_err(err, "GSL1DInterpolCR::evaluate (imaginary part):");
        res.imag(im_res);
    }

    return res;
};







GSL1DInterpolRR::tVec
GSL1DInterpolRR::restore_orig_x(){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolRR::restore_orig_x: workspace is not initialized");

    tVec out_x;
    if(T < 0){
        out_x = x_vec;
    }else{
        out_x.resize(x_vec.size() - 1);
        copy(x_vec.begin(), x_vec.end()-1, out_x.begin());
    }
    return out_x;
}

GSL1DInterpolCR::tVec
GSL1DInterpolCR::restore_orig_x(){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolCR::restore_orig_x: workspace is not initialized");

    tVec out_x;
    if(T < 0){
        out_x = x_vec;
    }else{
        out_x.resize(x_vec.size() - 1);
        copy(x_vec.begin(), x_vec.end() - 1, out_x.begin());
    }
    return out_x;
}







GSL1DInterpolRR::tVec
GSL1DInterpolRR::restore_orig_y(){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolRR::restore_orig_y: workspace is not initialized");

    tVec out_y;
    if(T < 0){
        out_y = y_vec;
    }else{
        out_y.resize(y_vec.size() - 1);
        copy(y_vec.begin(), y_vec.end()-1, out_y.begin());
    }
    return out_y;
}

GSL1DInterpolCR::tVec
GSL1DInterpolCR::restore_orig_y(){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolCR::restore_orig_y: workspace is not initialized");

    auto N = T < 0 ? x_vec.size() : x_vec.size() - 1;
    tVec out_y(2*N);

    for(int i = 0; i < N; i++){
        out_y[2*i] = re_y_vec[i];
        out_y[2*i + 1] = im_y_vec[i];
    }

    return out_y;
}






// todo: remove this! (it is used only in the SimpleScope, contradicts to the pure signals philosophy and
// todo: is incorrect for nonlinear approximations)

double
GSL1DInterpolRR::min_y(){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolRR::evaluate: workspace is not initialized");
    if(type != gsl_interp_linear)
        cerr << "GSL1DInterpolRR::min_y - for non linear interpolation minimum serach can return wrong result";

    return *std::min_element(y_vec.begin(), y_vec.end());
};

double
GSL1DInterpolRR::max_y(){
    if(!is_initialized())
        throw runtime_error("GSL1DInterpolRR::evaluate: workspace is not initialized");
    if(type != gsl_interp_linear)
        cerr << "GSL1DInterpolRR::min_y - for non linear interpolation maximum serach can return wrong result";

    return *std::max_element(y_vec.begin(), y_vec.end());
};

