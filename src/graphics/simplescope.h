//
// Created by morrigan on 1/31/17.
//

#ifndef COMPPHYSFWK_OSCILLOSOPE_HPP
#define COMPPHYSFWK_OSCILLOSOPE_HPP

#include <vector>
#include <cmath>
#include <ctime>
#include <string>

#include <gsl/gsl_interp.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "renderer.hpp"
#include "commons.h"
#include "gsl_1d_interpol.h"
#include "genoscdevice.hpp"

//TODO: add a packet that receives IContinuousSignal instead of point array

class SimpleScope : public Bitmap2DRenderer{

    //using tDevPtr = std::shared_ptr<IOscDevice>;
    using tDevPtr = std::shared_ptr<GenOscDevice<RealSignalPkt>>;

public:
    SimpleScope(tDevPtr src, size_t resolution);

    string getName(){return name;}
    void setName(string new_name){name = new_name;}

    size_t& getNx(){return Nx;};
    size_t& getNy(){return Ny;};

protected:
    virtual void draw_memory();
    virtual void msg_proc(const ALLEGRO_EVENT& ev);

protected:
    size_t Nx; //screen width (10)
    size_t Ny; //screen height (8)
    tDevPtr src; //this is a GenOscDevice
    std::vector<ALLEGRO_VERTEX> v;
    std::vector<double> scale; //scale factors
    GSL1DInterpolRR data; //data to be shown (need interpolation)

    bool mouse_down = false;
    bool save_next_frame = false;
    bool auto_adjust_scale = true; //best initial scale is estimated on first packet
    double cv = 0, ts = 0; //voltage center [volts], time start [sec]
    int v_scale_idx; //voltage scale [volts]
    int t_scale_idx; //time scale [sec]

    string name;
};

#endif //COMPPHYSFWK_OSCILLOSOPE_HPP
