//
// Created by morrigan on 6/18/18.
//

#ifndef DISTPIPELINEFWK_PELTIER_MODEL_H
#define DISTPIPELINEFWK_PELTIER_MODEL_H

#include <memory>
#include <list>
#include <tuple>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv2.h>

class PeltierModel{

    using tPtrStepperDriver = std::shared_ptr<gsl_odeiv2_driver>;

    // the evaluation result is stored in this way: Tin, Tout, time
    using tEvPoint = std::tuple<double, double, double>;

public:
    PeltierModel();
    void evaluate(double dt);


    // THE LAST PELTIER THERMOSTAT STATE
protected:
    // interior mass temperature
    double Tin_prev = 295.17; // [K] -> 22C = room temp

    // exterior mass temperature
    double Tout_prev = 295.17; // [K] -> 22C = room temp

    // current value of the control voltage
    double E_curr;


    // EVALUATION ROUTINES
private:

    // ODE system function
    static int model(double t, const double y[], double dydt[], void * params);

    // general GSL OrdDiffEqn system with arbitrary parameters
    gsl_odeiv2_system eq_system;

    // variable stepping size driver (for stability and speed)
    tPtrStepperDriver pDriver;



    // MODEL PARAMETERS (hardware equipment consts)
private:

    // heat capacity of the mass to be thermally stabilized (depends on mass and material) = interior mass
    double Cin = 52.7; // [J/K]

    // heat capacity of the cooler (heater) mass = exterior mass
    double Cout = 276; // [J/K]

    // thermal conductivity of the Peltier element
    double gamma = 0.562; // [W/K]

    // experimental thermal exchange parameter of  of the interior mass with the environment
    // (typically shell is minimized for thermal stability)
    // K_in_env = k01 * |Tenv - Tin| + k00;
    double k01 = 4e-5; // [W/(K^2)]
    double k00 = 5e-3; // [W/K]

    // experimental thermal exchange parameter of  of the exterior mass with the environment
    // (typically is maximized)
    // K_out_env = k11 * |Tenv - Tout| + k10;
    double k11 = 3.1e-3; // [W/K^2]
    double k10 = 3.88e-1; // [W/K]

    // Seebeck coefficient
    double alpha = 4.96e-2; // [V/K]

    // Peltier resistance
    double Rp = 2; // [Ohm]

    // Temperature of external environment
    double Tenv = (295.17); // [K] -> -> 22C = room temp

    // Thermal charge of a thermostat
    double Ph = 0; // [W]

    // Active resistance of the power source
    double Rap = 0; // [Ohm]
};

#endif //DISTPIPELINEFWK_PELTIER_MODEL_H
