//
// Created by morrigan on 10/11/18.
//

#ifndef DISTPIPELINEFWK_INTERFACE_INT_H
#define DISTPIPELINEFWK_INTERFACE_INT_H

#include <complex>
#include <vector>
#include <complex>

#include "../standards.h"

//TODO: add threadsafe access to evaluate(...)

/*
 * INTERPOLATION
 *
 * Interpolation of discrete data permits to fill missing points in-between measured ones that permit to work
 * with discrete data as it would be continuous function.
 *
 * In the case of interpolation, if the argument 'x' of the evaluate(x) function coincide with data point position,
 * all interpolation algorithms return an exact original data value. It means that any noise present in the signal will
 * be exactly reproduced. For this reason, it is not recommended to use non-linear interpolations for just acquired
 * data to avoid unpredictable effects, such as hi-order polynomial oscillations.
 *
 * In the "standards.h" header are defined two IContinuousXX interfaces, that are used for generic evaluation of
 * continuous functions. Both IInterpolXX inherit their main function - 'evaluate(x)' from those interfaces,
 * and at any moment can be upcasted to IContinuousXX to be used used in any computational routines.
 *
 * Note: different interpolations may construct their own data structures to optimize the evaluation process.
 * The original data is not stored without modification in the interpolate objects.
 */


 /*
  * This is the Base interface that is used in combination with one of the IContinuousXX to standardize
  * and document the main the interpolation classes functionality. None of the user defined classes or interfaces
  * shell use this interface for inheritance directly.
  */
struct IBaseInterpol {
    using tVec = std::vector<double>;

    /*
     * Setup or update data to be interpolated. The default x range is [0,1] and
     * the data is assumed to be not periodic and uniform. To set a nonelinear x-axis
     * it is required to specify the second parameter of this function. Both vectors need to address
     * size number of data points (y-data is packed). The x-data should be strictly increasing and not contain
     * repeating points.
     *
     * Note: In any case, after the first call to set_data(), the 'x' range is specified and is used for
     * all subsequent calls of set_data until:
     * 1. y-data size is changed
     * 2. the x-data is specified once again
     * 3. the clear() function was called
     */
    virtual void set_data(const tVec &y, const tVec &x = tVec()) = 0;

    /*
     * Check if the workspace was already initialized. Initialization mainly includes the x-range and
     * the other internal variables.
     */
    virtual bool is_initialized() = 0;

    /*
     * Periodical repetition of the signal by default sets it's x-range
     * to (-inf, +inf). If afterwards it is required to restrict the signal
     * definition area, it can be done via IDomain::set_interval() accessible from child interfaces.
     *
     * Note: In the case if 'x' data was specified in the 'set_data' function,
     * the 'y' samples distribution along x-axis are assumed to be not uniform.
     * In this case, it is possible to specify the missing distance
     * between x[N-1] and x[N] with custom 'dx'. If 'dx' it is not specified,
     * the linear x-axis is assumed and the 'dx' will be:
     *
     * dx = (x[N-1] - x[0])/N (default)
     *
     * The resulting period is always: T = x[N-1] - x[0] + dx.
     *
     * Note: once the signal is set to periodic, it can't be unset until the next cases:
     * 1. y-data size was changed (it invalidates x-data)
     * 2. x-data is specified once again (it invalidates 'dx')
     * 3. clear() function is called
     *
     * Warning: set_periodic can't be called twice. To check if the signal is already periodic
     * check if the function IDomain::get_period() returns positive value.
     */
    virtual void set_periodic(double dx = -1) = 0;

    /*
     * cleanup all data and return all members to their default values
     */
    virtual void clear() = 0;

    /*
     * Original data recovery in a generic case is not effective, however due to the main property of
     * interpolation - such recovery is absolutely possible. It can be used when efficiency is not critical.
     */
    virtual tVec restore_orig_x() = 0;
    virtual tVec restore_orig_y() = 0;

protected:
    /*
     * Any interpolated signals are continuous and can be repeated
     * periodically or not. If T == -1, the signal is not periodical and
     * any positive value means that a signal has a period.
     */
    double T = -1;

    /*
     * This interface is not for external usage, it standardize the IInterpolRR, IInterpolCR and possible any other
     * single variable interpolation interfaces that use packed multidimensional y-data.
     */
private:
    friend class GSL1DInterpolBase;
    friend struct IInterpolRR;
    friend struct IInterpolCR;

    IBaseInterpol(){};
};

struct IInterpolRR : public IBaseInterpol, public IContinuousRR {};
struct IInterpolCR : public IBaseInterpol, public IContinuousCR {};

#endif //DISTPIPELINEFWK_INTERFACE_INT_H