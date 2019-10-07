//
// Created by morrigan on 8/17/18.
//

#include <iostream>
#include <sstream>
#include <cmath>

using namespace std;

/*
 * In the DistPipelineFWK gnuplot is used as one of the main plotters. From C++ programs
 * it can be executed in two different ways:
 *
 * 1. As a console command
 * 2. As a child process
 *
 * Here we consider the first case, it is the most straightforward and the easiest one.
 * It consists of two steps:
 *
 * 1.1. Prepare gnuplot script ASCII text in the string buffer
 * 2.1. Send this buffer throug a pipe to the gnuplot input
 *
 * This approach has no limits on the complexity of the graphic to be plotted, however it is static
 * and slower than the second one. Another disadvantage is the requirement to prepare plotting string
 * completely and explicitly by a hand.
 */

int main(int argc, char** argv){

    //this command will be executed in the terminal and has the next construction:
    //echo "<gnuplot script>" | gnuplot --persist
    stringstream cmd;

    //an echo with open "
    cmd << "echo \"";

    //header of the gnuplot script
    cmd << "set terminal X11" << endl;
    cmd << "plot '-' with lines" << endl;

    //data
    for(double i = 0; i < 100; i++){
        cmd << i/50 << " " << sin(i/20) << endl;
    };

    cout << cmd.str() << endl;

    //tail of the terminal command
    cmd << "\" | gnuplot --persist";

    //executing
    system(cmd.str().c_str());

    return 0;
}

