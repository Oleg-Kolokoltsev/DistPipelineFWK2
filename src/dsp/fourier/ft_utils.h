//
// Created by morrigan on 10/11/18.
//

#ifndef DISTPIPELINEFWK_FT_UTILS_H
#define DISTPIPELINEFWK_FT_UTILS_H

#include <vector>

namespace ft{

    enum class SIGNAL_TYPE { REAL, COMPLEX };
    enum class SPECTRUM_TYPE {DFT_STANDARD, FT_STANDARD};

    /*
     * Direct shift of the DFT spectrum coefficients
     *
     * The basis functions of continuous FT are not discrete and therefore are not periodic
     * in frequency domain. For this reason the zero frequency is placed as usual in the middle
     * of the output vector. The concrete zero frequency index depends on the parity of N.
     *
     * Continuous FT of the real signal shell preserve the symmetry property:
     * X[center_idx + k] = Conjugate(X[center_idx - k]). In DFT when N is even, both parts of
     * the mirrored spectrum lobes have the same length. The positive lobe contains the signal
     * average at S[0] that is real and the negative lobe contains zero at X[N/2]. To preserve
     * this property, this hi-freq zero element is assumed to be the part of the negative lobe during
     * N/2 shift. Therefore, zero frequency position is defined as follows:
     *
     *   For N even: center_idx = N/2
     *   For N odd: center_idx = (N-1)/2
     *
     * Computation of the positive and negative frequencies is done by the following expression:
     *
     *   w_k = k - center_idx, where k = 0, ... , N-1.
     *
     * Also, to preserve parity of the complex conjugate coefficients of continuous FT when N is even,
     * the integration of the frequency w_0 (= 0 - center_idx) is not applyed and is set to zero manually.
     */

    // Fourier coefficients converter: DFT_STANDARD -> FT_STANDARD
    // signal type is always assumed to be COMPLEX packed data: {Re,Im,Re,Im...}
    void shift_dft_right(std::vector<double>&);

    // Fourier coefficients converter: FT_STANDARD -> DFT_STANDARD
    // signal type is always assumed to be COMPLEX packed data: {Re,Im,Re,Im...}
    void shift_dft_left(std::vector<double>&);

    // Map DFT indexes to the real world frequencies (depends on the standard used)
    std::vector<double> dft_freq_map(size_t N, double T, SPECTRUM_TYPE type);

    // print to console Fourier coefficients and corresponding frequency indexes
    // signal type is always assumed to be COMPLEX packed data: {Re,Im,Re,Im...}
    void print_dft(std::vector<double>&, SPECTRUM_TYPE);

    // returns the position of zero frequency depending on the spectrum storage
    size_t get_w0_idx(size_t, SPECTRUM_TYPE);


}

#endif //DISTPIPELINEFWK_FT_UTILS_H
