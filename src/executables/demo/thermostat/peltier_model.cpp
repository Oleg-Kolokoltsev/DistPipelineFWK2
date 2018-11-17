//
// Created by morrigan on 6/18/18.
//

#include <iostream>
#include <cmath>


#include <gsl/gsl_odeiv2.h>

#include "peltier_model.h"

using namespace std;

PeltierModel::PeltierModel(){

    // define the ODF system
    eq_system.function = model;
    eq_system.jacobian = nullptr;
    eq_system.dimension = 2;
    eq_system.params = this;

    // init driver with gsl_odeiv2_step_rkf45: Explicit embedded Runge-Kutta-Fehlberg
    auto driver_ptr = gsl_odeiv2_driver_alloc_y_new(&eq_system, gsl_odeiv2_step_rkf45, 1e-3, 1e-5, 1e-5);

    // release the driver automatically
    pDriver = tPtrStepperDriver(driver_ptr, [](gsl_odeiv2_driver* p) {gsl_odeiv2_driver_free(p);});
};

void
PeltierModel::evaluate(double dt){

    // reset from previous evaluation
    gsl_odeiv2_driver_reset(pDriver.get());

    // set the last temperature values
    double y[2] = {Tin_prev, Tout_prev};
    double t = 0;

    gsl_odeiv2_driver_apply(pDriver.get(), &t, dt, y);
    Tin_prev  = y[0];
    Tout_prev = y[1];
}


int PeltierModel::model(double t, const double y[], double dydt[], void * p){

    auto pte = static_cast<PeltierModel*>(p);

    // Tin -> y[0]; dTin / dt -> dydt[0]
    // Tout -> y[1]; dTout / dt -> dydt[1]

    // current through Peltier
    double I = pte->E_curr - pte->alpha*(y[1] - y[0]);
    I = I/(pte->Rp + pte->Rap);

    // interior mass thermal exchange with environment
    double K_in_env = pte->k01 * abs(pte->Tenv - y[0]) + pte->k00;

    // exterior mass thermal exchange with environment
    double K_out_env = pte->k11 * abs(pte->Tenv - y[1]) + pte->k10;

    // System of regular differential equations
    dydt[0] = pte->alpha*I*y[0] + 0.5*I*I*pte->Rp + pte->gamma*(y[1] - y[0]) + pte->Ph + K_in_env*(pte->Tenv - y[0]);
    dydt[0] = dydt[0]/pte->Cin;

    dydt[1] = -pte->alpha*I*y[1] + 0.5*I*I*pte->Rp + pte->gamma*(y[1] - y[0]) + K_out_env*(pte->Tenv - y[1]);
    dydt[1] = dydt[1]/pte->Cout;

    return GSL_SUCCESS;
}

