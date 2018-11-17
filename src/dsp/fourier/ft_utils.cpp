//
// Created by morrigan on 10/11/18.
//

#include <vector>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cmath>

#include "ft_utils.h"

using namespace std;

void
ft::shift_dft_right(vector<double>& data){

    if(data.size() % 2 != 0)
        throw runtime_error("shift_dft_right: data does not contain complex pairs");

    // N complex samples
    size_t N = data.size()/2;

    // 'pos' points on to the maximum negative frequency that has
    // to be shifted to data.begin() by std::rotate(...).
    int pos = N % 2 == 0 ? N/2 : (N+1)/2;

    // define middle poit assuming that vector contains complex pairs
    auto middle = data.begin();
    advance(middle, 2*pos);

    rotate(data.begin(), middle, data.end());
}

void
ft::shift_dft_left(std::vector<double>& data){
    if(data.size() % 2 != 0)
        throw runtime_error("shift_dft_right: data does not contain complex pairs");

    // N complex samples
    size_t N = data.size()/2;

    // 'pos' points on to the zero frequency that has to be shifted
    // data.begin() by std::rotate(...).
    auto pos = get_w0_idx(N, SPECTRUM_TYPE::FT_STANDARD);

    // define middle poit assuming that vector contains complex pairs
    auto middle = data.begin();
    advance(middle, 2*pos);

    rotate(data.begin(), middle, data.end());
}

vector<double>
ft::dft_freq_map(size_t N, double T, SPECTRUM_TYPE type){
    vector<double> w(N);

    if(type == SPECTRUM_TYPE::DFT_STANDARD){
        // maximum negative frequency idx
        auto n_idx = N % 2 == 0 ? N/2 : (N+1)/2;
        for(size_t i = 0; i < n_idx; i++)
            w[i] = ((double) i)*2*M_PI/T;
        for(size_t i = N-1; i >= n_idx; i--)
            w[i] = ((double)i-((double)N-1)-1)*2*M_PI/T;
    }else {
        // FT_STANDARD
        auto w0_idx = get_w0_idx(N, SPECTRUM_TYPE::FT_STANDARD);
        for(size_t i = 0; i < N; i++)
            w[i] = ((double)i - (double)w0_idx)*2*M_PI/T;
    }

    return move(w);
}

void ft::print_dft(std::vector<double>& data, SPECTRUM_TYPE type){
    if(data.size() % 2 != 0)
        throw runtime_error("shift_dft_right: data does not contain complex pairs");

    // N complex samples
    size_t N = data.size()/2;

    auto w0_idx = get_w0_idx(N, type);

    for(size_t i = 0; i < N; i++){
        cout << (int)((int)i - (int)w0_idx) << ": {" << data[2*i] << ", " << data[2*i+1] << "}" << endl;
    }
}

// N - is the number of complex points, so the index returned shell be multiplied by 2 when used
// TODO: or it is better to implement generic iterator for packed complex data?
size_t
ft::get_w0_idx(size_t N, SPECTRUM_TYPE type){
    if(type == SPECTRUM_TYPE::DFT_STANDARD){
        return 0;
    }else {
        // FT_STANDARD
        return N % 2 == 0 ? N/2 : (N-1)/2;
    }
}
