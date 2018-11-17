//
// Created by morrigan on 9/23/18.
//

#ifndef DISTPIPELINEFWK_PSMSGNSIG_TXT_H
#define DISTPIPELINEFWK_PSMSGNSIG_TXT_H

#include <map>

#include <gsl/gsl_matrix.h>

#include "psimsg.h"
/*
 * GENERAL NOTES
 *
 * With this message by default the next gnuplot command is called:
 *
 * --------------------------------------------------------------------------
 * plot \
 *  '-' with lines notitle,
 *  '-' with lines notitle
 *  ...
 *  <data1>
 *  e
 *  <data2>
 *  e
 *  ...
 * --------------------------------------------------------------------------
 *
 * Each data block, i.e. <data1>, <data2>, etc., is sent to gnuplot in text mode. This is
 * significantly slower than in binary mode, however, it can be used for debugging purposes
 * if something goes wrong. The fast message with the same functionality is PSMsgNSigBin. These
 * two message types are completely compatible, so it is possible to switch between them at any moment
 * by just changing the message type.
 *
 * To test gnuplot command output it is possible to prepare the message as it shell be sent to
 * GenOscDevice<PSMsgNSigTxt> device, and call get_data(..) function. Than you can print
 * the output string into the console and copy it to the gnuplot command line manually.
 * Do not forget to setup any terminal before testing, for example "set term X11" (Linux) or
 * "set term Qt" (Windows / Mac).
 *
 *
 * EXAMPLE
 *
 * -------------------------------------------------------------------------
 * plot \
 * '-' with lines notitle,\
 * '-' with lines notitle
 * 0 0.0193954
 * 1.5708 0.945437
 * 3.14159 0.599873
 * 4.71239 -0.593883
 * 6.28319 -0.93608
 * e
 * 0 0.912988
 * 1.5708 -0.141452
 * 3.14159 -0.949595
 * 4.71239 -0.487916
 * 6.28319 0.691316
 * e
 * --------------------------------------------------------------------------
 *
 *
 * DESCRIPTION
 *
 * 'mat' - Is a pointer to gsl_matrix with M rows and N columns (NSigTxt). Each column represent a signal value
 * for certain 'x' coordinate. In this message there is no special column for 'x' values. This column is
 * created automatically with an increment (x_max-x_min)/(N-1) starting from x_min for the
 * first row and ending at x_max for the last one. This way it is covered the most common case of
 * equidistant signal sampling for N signals. The advantage is that there is no need to prepare
 * an 'x' axis and send it each time to graphics device. So if you have an N-signals aquisition board
 * with the same signal sampling rate on each channel (that is the most common case for multichannel boards) -
 * the PSMsgNSig message is the simplest way to show all your signals in the same window and the best way
 * to start with plotscope messages. If you do not want to create the 'mat' pointer manually, it is possible
 * to use the second constructor: PSMsgNSigTxt(size_t rows, size_t cols).
 *
 *
 *
 * SIGNAL CUSTOMIZING
 *
 * 'with' - This is a map<int, string> that permits to specify any user defined properties line for any signal
 * separately. The map key is the index of the column in the 'mat' starting from 0. This map permits replace default
 * style "with lines notitle" after the '-' entry by any specific string following gnuplot specifications.
 * If your string is not acceptable by gnuplot, an error will occur, so test the output of get_data() in gnuplot
 * console in the case of any errors. Using 'with' map you can flexibly customize your data representation.
 * Make a dashed lines, specify colors, add legends etc. The data representation flexibility is in your hands
 * with this 'with' field.
 *
 *
 *
 * CONSTRAINTS
 *
 *  - The post processing drawings
 *  If you would like to show on the plot any special data, for example: discrete signal maxima with points, the
 *  PSMsgNSigTxt functionality will be not enough. This message does not permit to create sparse vectors.
 *
 *
 *  - Same Y-scale
 *  All signals in matrix have the same y-axis, so signal specific scale as in case of any hardware graphics
 *  is not supported by this message type.
 *
 *  - Plot type
 *  The 'plot' function is predefined here and can't be changed.
 *
 *  - User actions
 *  The user actions (scale, shift) are predefined.
 *
 * You can use PSMsgNSigTxt as the base class if you need to correct something.
 */

struct PSMsgNSigTxt : public PSIMsg{

    // an empty constructor does nothing, it is
    // useful if you already have the a gsl_matrix initialized before (be sure your smart pointer has a destructor)
    PSMsgNSigTxt(){}

    // creates a new matrix pointer correctly (can be inefficient if you already have this matrix)
    PSMsgNSigTxt(size_t rows, size_t cols);

    // copy constructor
    PSMsgNSigTxt(const PSMsgNSigTxt& msg);

    // generates a gnuplot command based on the structure field values
    virtual void get_data(std::string& dest) const;

    // required
    // cols - set of signals
    // rows - equidistant points x in [x_min, x_max] where dx = (x_max - x_min)/(N-1)
    std::shared_ptr<gsl_matrix> mat;

    // optional, can be defined or not for any signal
    // with[col_idx] is the string that will replace the default "with lines notitle" without any verifications
    std::map<int, std::string> with;

    // required
    // this is sampling rate => dx = (x_max - x_min)/(M-1), where M is the number of raws in "mat"
    double x_min = 0.0, x_max = 0.0;
};

#endif //DISTPIPELINEFWK_PSMSGNSIG_TXT_H
