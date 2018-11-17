//
// Created by morrigan on 19/12/17.
//

#ifndef DISTPIPELINEFWK_COMMONS_H_H
#define DISTPIPELINEFWK_COMMONS_H_H

#include <string>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include <algorithm>

#include <boost/filesystem.hpp>

namespace osc_comm {

    int get_best_scale(double d_min, double d_max, std::vector<double> scale, double mult);

    //replace '~' with current user home path (stolen by someone from python sources)
    std::string expand_user(std::string path);

    void save_data_file(std::string file_path, const std::vector<double>& x, const std::vector<double>& y);

    void save_data(std::string osc_name, const std::vector<double>& x, const std::vector<double>& y);
}

#endif //DISTPIPELINEFWK_COMMONS_H_H
