//
// Created by morrigan on 8/17/18.
//

#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;

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

    //tail of the terminal command
    cmd << "\" | gnuplot --persist";

    //executing
    system(cmd.str().c_str());

    return 0;
}

