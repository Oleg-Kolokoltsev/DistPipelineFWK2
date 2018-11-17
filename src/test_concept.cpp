//
// Created by morrigan on 9/26/18.
//

#include <iostream>

#include <boost/numeric/interval.hpp>

#include "gsl_1d_interpol.h"

using namespace std;

int main(int argc, char** argv){

    GSL1DInterpolRR interpol, interpol2;

    vector<double> x, y;
    for(int i = 0; i < 10; i++)
        y.push_back(i), x.push_back(i-5);

    interpol.set_data(y, x);
    interpol.set_periodic();

    for(int i = 0; i < 10; i++)
        cout << x[i] << endl;

    interpol2 = interpol;

    for(double x = -100; x <= 100; x += 1)
        cout << x << " " << interpol.evaluate(x) << endl;

}