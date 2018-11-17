//
// Created by morrigan on 9/30/18.
//

#include <exception>
#include <functional>
#include <iostream>

#include "analytic_num_ft.h"

using namespace std;

AnalyticNumFT::AnalyticNumFT(size_t n){
    // todo: GSL_ETABLE if n is insuficcient for requested accuracy
    auto p_ws = gsl_integration_workspace_alloc(n);
    ws = tPtrWs(p_ws, [](tPtrWs::element_type* p_ws){gsl_integration_workspace_free(p_ws);});

    // todo: is the 'limit' correct?
    auto p_cos = gsl_integration_qawo_table_alloc(0.0, 1.0, GSL_INTEG_COSINE, ws->limit);
    auto p_sin = gsl_integration_qawo_table_alloc(0.0, 1.0, GSL_INTEG_SINE, ws->limit);

    qawo_cos = tQAWOTable(p_cos, [] (tQAWOTable::element_type* p){gsl_integration_qawo_table_free(p);});
    qawo_sin = tQAWOTable(p_sin, [] (tQAWOTable::element_type* p){gsl_integration_qawo_table_free(p);});
}

AnalyticNumFT::tCmplx
AnalyticNumFT::integrate(IContinuousRR* func, double a, double L, double w){

    // initialize GSL function
    gsl_function_struct gsl_func;

    gsl_func.params = func;
    gsl_func.function = [](double x, void* params) -> double {
        auto func = static_cast<IContinuousRR*>(params);
        return func->evaluate(x);
    };

    gsl_integration_qawo_table_set(qawo_cos.get(), w, L, GSL_INTEG_COSINE);
    gsl_integration_qawo_table_set(qawo_sin.get(), w, L, GSL_INTEG_SINE);

    double sin_int, cos_int;
    double abserr;

    gsl_integration_qawo(&gsl_func, a, epsabs, epsrel, ws->limit, ws.get(), qawo_cos.get(), &cos_int, &abserr);
    gsl_integration_qawo(&gsl_func, a, epsabs, epsrel, ws->limit, ws.get(), qawo_sin.get(), &sin_int, &abserr);

    return tCmplx(cos_int/L, sin_int/L);
}
