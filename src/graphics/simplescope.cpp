//
// Created by morrigan on 9/24/18.
//

#include "simplescope.h"

using namespace std;

SimpleScope::SimpleScope(tDevPtr src, size_t resolution = 40) :
Bitmap2DRenderer(), src{src}, Nx{10*resolution}, Ny{8*resolution} {

    v.resize(Nx);
    memset(v.data(), '\0', Nx*sizeof(ALLEGRO_VERTEX));
    for(int i = 0; i < Nx; i++) v[i].x = i;

    v_scale_idx = 0;
    t_scale_idx = 0;

    for(int i = -12; i <= 12; i++){
        scale.push_back(1*pow(10,(float)i));
        scale.push_back(2*pow(10,(float)i));
        scale.push_back(5*pow(10,(float)i));
        if(i < 0) v_scale_idx += 3;
        if(i < -7) t_scale_idx +=3;
    }
}

void
SimpleScope::draw_memory(){

    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
    al_clear_to_color(al_map_rgba(0, 0, 0, 50));
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);

    //draw grid
    for(int i = 1; i <= 9; i++) al_draw_line(i*Nx/10,0,i*Nx/10,Ny,al_map_rgb(50,50,50),1);
    for(int i = 1; i <= 7; i++) al_draw_line(0,i*Ny/8,Nx,i*Ny/8,al_map_rgb(50,50,50),1);

    //zero-level line
    al_draw_line(0, ((double)Ny/2+cv),Nx,((double)Ny/2+cv),al_map_rgb(200,200,255),2);

    //get the data
    auto in_msg = src->get_frame();
    if(!in_msg || in_msg->data.empty()) return;

    data.set_data(move(in_msg->data));
    in_msg.reset();

    //save data if requested
    if(save_next_frame) {
        todo: osc_comm::save_data(name, data.restore_orig_x(), data.restore_orig_y());
        save_next_frame = false;
    }

    //continuous limits (for interpolation)
    const double x_min = data.get_interval().lower();
    const double x_max = data.get_interval().upper();

    //estimate best scale on first packet
    if(auto_adjust_scale){
        v_scale_idx = osc_comm::get_best_scale(data.min_y(), data.max_y(), scale, 8.0);
        t_scale_idx = osc_comm::get_best_scale(x_min, x_max, scale, 10.0);
        auto_adjust_scale = false;
    }



    //discrete limits (for al_draw_prim)
    int start = 0, end = Nx;

    float y;
    for(double i = 0; i < Nx; i++){

        //transform from OSC coordinate to physical unit
        double x = 10.0*scale[t_scale_idx]*(i - ts)/(double)Nx;
        y = -1;

        if(x < x_min){
            start++;
        }else if(x > x_max){
            end--;
        }else{
            y = data.evaluate(x);

            //transform from physical units to OSC coordinate
            y = (double)Ny/2 + cv - y*(double)Ny/(8.0*scale[v_scale_idx]);
        }

        v[i].y = y;
        v[i].color = al_map_rgb(0,255,0);
    }

    if(start < end)
        al_draw_prim(v.data(), nullptr, nullptr, start, end, ALLEGRO_PRIM_LINE_STRIP);

}


void
SimpleScope::msg_proc(const ALLEGRO_EVENT& ev){
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
        mouse_down = true;
    }else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP){
        mouse_down = false;
    }else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES){
        if(mouse_down) {
            cv += ev.mouse.dy;
            ts += ev.mouse.dx;
        }
    }else if(ev.type == ALLEGRO_EVENT_KEY_DOWN){
        if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN){
            if(v_scale_idx > 0)v_scale_idx--;
        }else if(ev.keyboard.keycode == ALLEGRO_KEY_UP){
            if(v_scale_idx < scale.size()-1)v_scale_idx++;
        }else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT){
            if(t_scale_idx > 0)t_scale_idx--;
        }else if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
            if(t_scale_idx < scale.size()-1)t_scale_idx++;
        }else if(ev.keyboard.keycode == ALLEGRO_KEY_S){
            save_next_frame = true;
        }
    }
}

