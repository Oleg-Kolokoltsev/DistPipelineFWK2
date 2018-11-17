//
// Created by morrigan on 10/14/18.
//

#include <iostream>

#include <gsl/gsl_errno.h>

#include "gsl_1d_interpol.h"

using namespace std;

void
GSL1DInterpolBase::x_autofill(size_t N){
    x_vec.resize(N);

    const double dx = 1.0/(double)((double)N - (double)1.0);
    double x = 0;

    for_each(x_vec.begin(), x_vec.end(), [&x, &dx](tBVec::value_type& v){
        v = x;
        x += dx;
    });

    // ensure there is no numeric errors
    x_vec[N-1] = (double) 1.0;
}

void
GSL1DInterpolBase::update_ws(tPtrWork& ws, const tBVec& y){
    if(!ws) {
        ws = tPtrWork(gsl_interp_alloc(type, y.size()), [](tPtrWork::element_type *p_ws) {
            gsl_interp_free(p_ws);
        });
    }

    alert_gsl_err(gsl_interp_init(ws.get(), x_vec.data(), y.data(), x_vec.size()),"::update_ws");

    if(!accel) {
        accel = tPtrAccel(gsl_interp_accel_alloc(), [](tPtrAccel::element_type *p_accel) {
            gsl_interp_accel_free(p_accel);
        });
    }else{
        gsl_interp_accel_reset(accel.get());
    }
}

void
GSL1DInterpolBase::alert_gsl_err(int err, string msg){
    if(err != GSL_SUCCESS) {
        stringstream ss;

        if(!msg.empty()){
            ss << msg << " gsl error code = " << err;
        }else{
            ss << "GSL1DInterpolBase: GSL error code = " << err;
        }

        throw runtime_error(ss.str());
    }
}