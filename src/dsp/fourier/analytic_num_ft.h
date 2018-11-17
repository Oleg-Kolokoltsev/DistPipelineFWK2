//
// Created by morrigan on 9/30/18.
//

#ifndef DISTPIPELINEFWK_ANALYTIC_NUM_FT_H
#define DISTPIPELINEFWK_ANALYTIC_NUM_FT_H

#include <cstdlib>
#include <vector>
#include <map>
#include <memory>
#include <utility>
#include <complex>

#include <gsl/gsl_integration.h>

#include "../standards.h"

// http://people.duke.edu/~hpgavin/cee541/Fourier.pdf

/*
 * NUMERICAL CALCULATION OF THE ANALYTIC FOURIER TRANSFORM
 *
 * INPUT:
 * Numerical integration is defined for continuous functions only. Unlike the analytical
 * signal functions, in the case of a discrete signals they need to be interpolated first.
 * Uniform access to both of these signal types is done via IContinuousXX interfaces
 * defined in the i_interpolation.h. Any signal can be either real or complex, so there are
 * two interfaces: IContinuousRR or IContinuousCR respectively.
 *
 * In the case of discrete interpolated signal it is assumed that a discretization was applied
 * to the finite band signal with sampling frequency Fs > 2Fb, where Fb - is the maximum frequency
 * present in the original continuous signal. If this criteria is not satisfied,
 * the spectrum overlap may be observed.
 *
 * The input signal can be periodic or aperiodic (finite), and for both it is defined an interval of their
 * argument within which the signal is defined. In the case of periodic signal it is assumed that this interval
 * is a signal period. For aperiodic finite signal it is assumed that outside of this interval the
 * function is simply zero.
 *
 * UNITS:
 * todo: ????... well as long as function interval is always defined - probably no matter.
 *
 *
 * DIRECT TRANSFORM:
 *
 * Direct continuous Fourier transform computes spectral density for finite signals or
 * the Fourier integral over the period for periodical signals. As long as input
 * function is always continuous, it is possible to compute Fourier coefficients for any subset of
 * frequencies. Hence, depending on the transform parameters the inverse transform may produce a
 * signal that differ from the original one.
 *
 * todo: check what happens if a LO pass filter is appliyed on the finite signal.
 * todo: check the case when direct fourier spectrum is an exact representation of the input signal
 *
 * Output signal types:
 *
 *   1. The vector of Fourier coefficients taken on equidistant frequencies that coincide with
 *   N/2 shifted DFT fourier spectrum (see ft_utils.h for complete description).
 *
 *   2. The vector of Fourier coefficients computed for any given array of frequencies.
 *
 *   3. The IContinuousCR object (for finite signals)
 *
 *   4. Single coefficient for the given frequency (for example - peak search on continuous spectra)
 *
 * To produce the selected output the next integral is appliyed for each frequency:
 *
 * Integrate[-T/2, T/2, f(x) * exp(-j wq t) dt] / T;
 *
 * todo: add 1-point extension for interpolated periodic signals to reach periodicity
 */

class AnalyticNumFT {

    using tCmplx = std::complex<double>;
    using tVec = std::vector<double>;
    using tQAWOTable = std::shared_ptr<gsl_integration_qawo_table>;
    using tPtrWs = std::shared_ptr<gsl_integration_workspace>;

public:

    // constructor (65k subintervals by default)
    AnalyticNumFT(size_t n = 16);

    // integrate single 'w' (no parallel processes will be created)
    tCmplx integrate(IContinuousRR* func, double a, double L, double w);

    // integrate multiply 'w' (parallel computation: todo: make sure your function is parallel)
    // ... - just this difference

    // also it is possible to work with Laplace in this way if w is complex!

private:
    tQAWOTable qawo_sin;
    tQAWOTable qawo_cos;

    tPtrWs ws;

public:
    double epsabs = 1.0e-6; // absolute integration error
    double epsrel = 1.0e-6; // relative integration error
};


#endif //DISTPIPELINEFWK_ANALYTIC_NUM_FT_H
