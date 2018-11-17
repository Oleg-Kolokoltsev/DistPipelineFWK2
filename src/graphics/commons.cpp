//
// Created by morrigan on 9/24/18.
//

#include "commons.h"

using namespace std;

namespace osc_comm {

    int get_best_scale(double d_min, double d_max, std::vector<double> scale, double mult){
        //graphics has 10 cells for x and 8 cells for y (mult factor)
        //the scale is given in seconds/volts per 1 cell
        //D is a data band in real units for 1 cell along x or y direction
        double D = (d_max - d_min)/mult;

        //search for the nearest scale less than 'D'
        auto idx = std::lower_bound(scale.begin(),scale.end(),D) - scale.begin();
        if(idx >= scale.size()) return scale.size()-1;

        //check prev scale is better than idx
        if(idx > 0){
            return std::abs(D - scale[idx]) < std::abs(D - scale[idx-1]) ? idx : (idx-1);
        }else{
            //that was the last scale
            return idx;
        }
    }

    //replace '~' with current user home path (stolen from python sources)
    std::string expand_user(std::string path) {
        if (not path.empty() and path[0] == '~') {
            assert(path.size() == 1 or path[1] == '/');  // or other error handling
            char const* home = getenv("HOME");
            if (home or ((home = getenv("USERPROFILE")))) {
                path.replace(0, 1, home);
            }
            else {
                char const *hdrive = getenv("HOMEDRIVE"),
                        *hpath = getenv("HOMEPATH");
                assert(hdrive);  // or other error handling
                assert(hpath);
                path.replace(0, 1, std::string(hdrive) + hpath);
            }
        }
        return path;
    }

    void save_data_file(std::string file_path, const std::vector<double>& x, const std::vector<double>& y){
        //save formatted data
        std::ofstream file;
        file.open(file_path);

        for(int i = 0; i < x.size(); i++){
            file << std::right
                 << std::setfill(' ')
                 << std::setprecision(5)
                 << std::setw(12)
                 << std::scientific
                 << x[i];

            file << std::right
                 << std::setfill(' ')
                 << std::setprecision(5)
                 << std::setw(14)
                 << std::scientific
                 << y[i] << '\n';
        }

        file.close();
    }

    void save_data(std::string osc_name, const std::vector<double>& x, const std::vector<double>& y) {

        //get current time
        time_t rawtime;
        struct tm *timeinfo;
        char buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        //construct complete file path
        strftime(buffer, sizeof(buffer), "%d-%m-%Y_%I-%M-%S", timeinfo);
        std::string file_path("~/OscilloscopeSnapshots/");
        file_path = expand_user(file_path);
        if(!osc_name.empty()){
            file_path.append(osc_name);
            file_path.append("_");
        }
        file_path.append(buffer);
        file_path.append(".dat");

        //create directory if not exist
        auto dir = boost::filesystem::path(file_path.c_str()).parent_path();
        if(boost::filesystem::create_directory(dir))
            std::cout << "New directory created: " << dir.generic_string() << std::endl;

        //save formatted data
        save_data_file(file_path, x, y);
        std::cout << "file created: " << file_path << std::endl;
    }
}

