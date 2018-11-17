//
// Created by morrigan on 9/24/18.
//

#include <iostream>
#include <sstream>

#include "psmsgnsig_txt.h"

using namespace std;


// creates a mew matrix with destructor
PSMsgNSigTxt::PSMsgNSigTxt(size_t rows, size_t cols){
    // automatic gsl_matrix allocation
    mat = shared_ptr<gsl_matrix>(gsl_matrix_alloc(rows, cols),
                                 [] (gsl_matrix *p) {gsl_matrix_free(p);} );

    // for any case setting all matrix elements to zero
    gsl_matrix_set_zero(mat.get());
}

// copy constructor
PSMsgNSigTxt::PSMsgNSigTxt(const PSMsgNSigTxt& msg) : PSIMsg(msg){

    if(msg.mat){
        this->mat = shared_ptr<gsl_matrix>(gsl_matrix_alloc(msg.mat->size1, msg.mat->size2),
                                           [] (gsl_matrix *p) {gsl_matrix_free(p);} );
        gsl_matrix_memcpy(this->mat.get(), msg.mat.get());
    }

    this->with = msg.with;
    this->x_min = msg.x_min;
    this->x_max = msg.x_max;
}

/*
 * Plain text data transfer.
 * dest - an empty string that is filled in gnuplot format using already initialized PSMsgNSigTxt fields
 */
void
PSMsgNSigTxt::get_data(string& dest) const{
    dest.clear();
    if(!mat) throw runtime_error("PlotScopeFrame: matrix not initialized");
    if(x_min >= x_max) runtime_error("PlotScopeFrame: invalid X dimensions");

    auto n_rows = mat->size1;
    auto n_cols = mat->size2;
    if(!n_rows || !n_cols || n_rows < 2) throw runtime_error("PlotScopeFrame: matrix has no rows or columns");

    stringstream ss;
    ss << "plot \\" << endl;
    for(int i = 0; i < n_cols; i++) {
        ss << " '-' ";
        if(with.count(i)){
            ss << with.at(i);
        }else{
            ss << "with lines notitle";
        }
        ss << ((i == n_cols-1) ? "\n" : ",\\\n");
    }

    for(int i = 0; i < n_cols; i++){
        for(double j = 0; j < n_rows; j++){
            ss << j*(x_max - x_min)/((double)n_rows - 1.0) << " ";
            ss << gsl_matrix_get(mat.get(), j, i) << "\n";
        }
        ss << "e\n";
    }
    ss << "\n";

    dest = ss.str();
}

