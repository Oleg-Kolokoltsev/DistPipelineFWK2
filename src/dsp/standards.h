//
// Created by morrigan on 10/13/18.
//

#ifndef DISTPIPELINEFWK_STANDARDS_H
#define DISTPIPELINEFWK_STANDARDS_H

#include <complex>

#include <boost/numeric/interval.hpp>

/*
 * The real world signals are continuous. However, there are techniques, such as interpolation, fitting, extrapolation,
 * etc. that permit to create continuous functions based on discrete data. Continuous signals can be finite or
 * infinite. It is possible to model any signal type, that means that a signal can be defined even on infinite or a
 * half-infinite domain.
 *
 * The interface IFuncDomain, by using the Boost 'interval' data type, specify the domain where a single variable
 * function values can be accessed, so this is a restriction to the specific function region. The 'interval' object
 * supports any intervals, including infinite. Also, if a given function is defined everywhere it is possible to make
 * it's cut and restrict it to a specific region.
 *
 * Exist two types of interval boundaries: closed '[' and open '('. Interpretation of the open interval is
 * implementation specific. It can indicate on the presence of any kind of singularity, on a critical point,
 * or can be used for interval arithmetic that is fully supported by the Boost interval library. The interval
 * arithmetic opens the way for implementation of the special class of interval algorithms. A simple example of such
 * algorithm is the interval Newton method, which guarantee the localization of ALL zeros within the given region.
 *
 * A special group of infinite signals are the periodic signals. In the case if any function or signal repeats
 * periodically within (-inf, inf), the function 'get_period()' brings an information about that period. In the case
 * that a function is not periodic, the 'get_period' must return -1. In the case of periodic functions, the finite or
 * half finite intervals permit to slice that function just as it is done for any other functions whatever they are
 * finite or not. Therefore it is important not to interpret the periodic function interval extent as it's period.
 */


struct IDomain {

    // positive and negative infinity constants
    static constexpr double p_inf = std::numeric_limits<double>::infinity();
    static constexpr double n_inf = -std::numeric_limits<double>::infinity();

    using tInterval = boost::numeric::interval<double>;

    // this function is implementation specific and must return -1 if there is no periodicity
    virtual double get_period() = 0;


    // search boost/numeric/interval/utility.hpp for the functions that operate with intervals
    const tInterval& get_interval() const {return interval;}

    // not all implementations permit to assign arbitrary intervals
    virtual void set_interval(tInterval ivl) = 0;

protected:
    tInterval interval;// = tInterval::empty(); <= boost::interval: empty interval created ???

private:

    /*
     * This interface is not for external usage, it standardize the IContinuousRR and IContinuousCR and
     * any future interfaces for continuous single variable functions.
     */
    friend struct IContinuousRR;
    friend struct IContinuousCR;
    IDomain(){};
};


/*
 * The most commonly used signals are the real function of Real argument, the IContinuousRR, and the complex
 * function of real argument - IContinuousCR. Both are defined within some interval and/or can have periodicity,
 * so both inherit the IDomain interface. Separation of continuous signals on two groups is evident from the point
 * of view of code optimization. In particular, the metaprogramming techniques permit configure generic routines
 * at the compile time based on the type of a concrete interface.
 */
struct IContinuousRR : public IDomain{


    virtual double evaluate(double x) = 0;

    //TODO: define copy function!
    /*
     * This is required to grant possibility to copy upcasted objects of real implementations.
     * It is useful because the 'evaluate' function can't be 'const' to give possibility to change the
     * object data during evaluation that is necessary for some optimizations. So 'evaluate' is not thread
     * safe, and if we want to access to the same object from different threads the only possibility is
     * to copy that object.
     */

};

struct IContinuousCR : public IDomain{
    // It is a frequent case when there is just a real or an imaginary part of the complex value needs
    // to be calculated. In many algorithms the complex value parts are computed separately. The ZPART
    // enumeration define which part of the complex return value shell be computed by evaluate function.
    enum ZPART {RE, IM, CPLX};

    virtual std::complex<double> evaluate(double x, ZPART z_part = CPLX) = 0;

    //TODO: define copy function!
    // (Same reason)

};

#endif //DISTPIPELINEFWK_STANDARDS_H
