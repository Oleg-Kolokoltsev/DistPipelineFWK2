//
// Created by morrigan on 10/12/18.
//

#ifndef DISTPIPELINEFWK_GSL_1D_INTERPOL_H
#define DISTPIPELINEFWK_GSL_1D_INTERPOL_H

#include <memory>
#include <vector>

#include <gsl/gsl_interp.h>

#include "i_interpol.h"

//http://folk.uio.no/in329/nchap5.pdf - approximation/interpolation

class GSL1DInterpolBase{

    using tBVec = IBaseInterpol::tVec;

public:

    using tPtrWork = std::shared_ptr<gsl_interp>;
    using tPtrAccel = std::shared_ptr<gsl_interp_accel>;

    GSL1DInterpolBase(const gsl_interp_type* type = gsl_interp_linear) : type(type){}

protected:
    void update_ws(tPtrWork& ws, const tBVec& y);
    void x_autofill(size_t N);
    void alert_gsl_err(int, std::string = "");

protected:
    tBVec x_vec;
    tPtrAccel accel;
    const gsl_interp_type* type;
};

// Real signals
class GSL1DInterpolRR : public GSL1DInterpolBase, public IInterpolRR{
public:
    using tBase = GSL1DInterpolBase;

    // regular constructor
    GSL1DInterpolRR(const gsl_interp_type* type = gsl_interp_linear);

    // copy constructor
    GSL1DInterpolRR(const GSL1DInterpolRR& obj);

    virtual void set_data(const tVec &y, const tVec &x = tVec());
    virtual double evaluate(double);

    virtual void set_periodic(double dx = -1);
    virtual void set_interval(tInterval ivl);

    virtual tVec restore_orig_x();
    virtual tVec restore_orig_y();
    virtual bool is_initialized(){return !x_vec.empty();};
    double get_period(){return T;};
    virtual void clear(){T = -1; x_vec.clear(); y_vec.clear(); ws.reset(); accel.reset();};

    //TODO: remove this
    double min_y();
    double max_y();

private:
    tVec y_vec;
    tPtrWork ws;
};

// Complex signals
class GSL1DInterpolCR : public GSL1DInterpolBase, public IInterpolCR{
public:
    using tBase = GSL1DInterpolBase;

    // regular constructor
    GSL1DInterpolCR(const gsl_interp_type* type = gsl_interp_linear);

    // copy constructor
    GSL1DInterpolCR(const GSL1DInterpolCR& obj);

    virtual void set_data(const tVec &y, const tVec &x = tVec()); // y is a packed complex data
    virtual std::complex<double> evaluate(double x, ZPART z_part = CPLX);

    virtual void set_periodic(double dx = -1);
    virtual void set_interval(tInterval ivl);
    virtual bool is_initialized(){return !x_vec.empty();};
    virtual tVec restore_orig_x();
    virtual tVec restore_orig_y(); // returns packed complex vector

    double get_period(){return T;};
    virtual void clear(){T = -1; x_vec.clear(); re_y_vec.clear(); im_y_vec.clear(); re_ws.reset(); im_ws.reset(); accel.reset();}

private:
    tVec re_y_vec;
    tVec im_y_vec;

    tPtrWork re_ws;
    tPtrWork im_ws;
};

#endif //DISTPIPELINEFWK_GSL_1D_INTERPOL_H
