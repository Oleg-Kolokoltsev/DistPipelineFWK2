//
// Created by morrigan on 6/19/18.
//

#ifndef DISTPIPELINEFWK_SYS_MONITOR_H
#define DISTPIPELINEFWK_SYS_MONITOR_H

#include <list>

#include <gsl/gsl_interp.h>
#include <gsl/gsl_bspline.h>
#include <gsl/gsl_multifit.h>

#include "base_node.hpp"
#include "messages.h"
#include "pid_filter.hpp"

class SysMonitorFilter : public BaseNode<PeltierStateMsg>{
public:
    using tBase = BaseNode<PeltierStateMsg>;
    using tPtrIn = BaseNode::tPtrIn;
    using tPtrPIDMsg = std::shared_ptr<PIDMsgIn>;
    using tPtrSignalPktMsg = std::shared_ptr<RealSignalPkt>;

    using tPtrOsc = std::shared_ptr<BaseNode<RealSignalPkt>>;
    using tPtrPID = std::shared_ptr<BaseNode<PIDMsgIn>>;

    enum class MODES {Calibrate, Operate};

protected:
    friend class NodeFactory;
    SysMonitorFilter();

    virtual bool process_usr_msg(tPtrIn&& msg);

    void monitor_state(tPtrIn&& msg);
    void interpolate();
    double fast_extrapolate(double);

    void save_calibration();
    bool read_calibration();

    // initialize required target nodes manually from main
public:
    tPtrOsc scope_Err;
    tPtrOsc scope_U;

    tPtrPID PIDController;

private:
    // Number of the last samples to be sent to oscilloscopes
    const double N = 300; // [sec]

    std::list<PeltierState> loop_list;

    // Limits of the control parameter
    double U_min = -0.5;//0;
    double U_max = 0.5;//255;

    // Always start from calibration
    MODES mode = MODES::Calibrate;

    // calibration variables
    double val;
    std::list<double> loop_buffer;
    int loop_buffer_sz = 30;

    struct TU{ double T, U; };
    struct by_T {
        bool operator()(TU const &a, TU const &b) {
            return a.T < b.T;
        }
    };

    std::vector<TU> measured;

    // interpolation
    std::vector<double> T;
    std::vector<double> U;

    std::shared_ptr<gsl_interp> interp;
    std::shared_ptr<gsl_interp_accel> acc;

    double T_min, T_max;
};

#endif //DISTPIPELINEFWK_SYS_MONITOR_H
