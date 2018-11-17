/*
 * renderer.hpp
 *
 *  Created on: Nov 1, 2016
 *      Author: morrigan
 */

#ifndef SRC_LIB_VISUAL_RENDERER_HPP_
#define SRC_LIB_VISUAL_RENDERER_HPP_

#include <memory>
#include <thread>
#include <atomic>
#include <functional>

#include <allegro5/allegro.h>

using namespace std;

class IRenderer {
public:
	virtual ~IRenderer(){};
	virtual void render() = 0;
	virtual void attach() = 0;
	virtual void detach() = 0;
	virtual size_t& getNx() = 0;
	virtual size_t& getNy() = 0;
	virtual void msg_proc(const ALLEGRO_EVENT& ev){};
};

class Bitmap2DRenderer : public IRenderer{
public:
	using tBase = IRenderer;
	using tBitmapPtr = unique_ptr<ALLEGRO_BITMAP,std::function<void(ALLEGRO_BITMAP*)>>;
	using tThreadPtr = unique_ptr<thread>;

	Bitmap2DRenderer(){
		update_req = false;
		terminate = true;
	};

	~Bitmap2DRenderer(){ detach(); }

	//shell be called after display creation
	virtual void attach(){

		Nx = getNx();
		Ny = getNy();

		curr_display = al_get_current_display();

		cx = al_get_display_width(curr_display);
		cy = al_get_display_height(curr_display);

		if(cx/Nx > cy/Ny){
			aspect = (double)cy/Ny;
			dy = 0;
			dx = (cx - aspect*Nx)/2;
		}else{
			aspect = (double)cx/Nx;
			dy = (cy - aspect*Ny)/2;
			dx = 0;
		}

		al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
		video_bmp = tBitmapPtr(al_create_bitmap(cx,cy),[=](ALLEGRO_BITMAP* ptr){al_destroy_bitmap(ptr);});

		al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
		mem_bmp = tBitmapPtr(al_create_bitmap(Nx, Ny),[=](ALLEGRO_BITMAP* ptr){al_destroy_bitmap(ptr);});

		terminate = false;
		if(mem_update_thread == nullptr || !mem_update_thread->joinable())
			mem_update_thread.reset(new thread(Bitmap2DRenderer::mem_update_proc, this));
	};

	virtual void detach(){
		terminate = true;
		if(mem_update_thread && mem_update_thread->joinable())
			mem_update_thread->join();
		mem_bmp = nullptr;
		video_bmp = nullptr;
	}

	void render(){
		params_guard();
		if(!mem_bmp || !video_bmp || !curr_display) return;
		if(update_req){
			al_draw_bitmap(video_bmp.get(), 0, 0, 0);
		}else{
			al_set_target_bitmap(video_bmp.get());
			if(Nx == cx && Ny == cy){
				al_draw_bitmap(mem_bmp.get(), 0, 0, 0);
			}else{
				al_clear_to_color(al_map_rgb(100, 100, 100));
				al_draw_scaled_bitmap(mem_bmp.get(), 0, 0, Nx, Ny, dx, dy, aspect*Nx, aspect*Ny,0);
			}

			draw_video();

			al_set_target_bitmap(al_get_backbuffer(curr_display));
			al_draw_bitmap(video_bmp.get(), 0, 0, 0);

			update_req = true;
		}
		al_flip_display();
	}
	
protected:

	virtual void draw_memory(){}; //fast, before scale (for general drawing and data acquisition)
	virtual void draw_video(){}; //slow, after scale (for interface elements only)

private:
	static void mem_update_proc(Bitmap2DRenderer* p_this){
		while(1){
			if(p_this->terminate) break;
			if(p_this->update_req){
				al_set_target_bitmap(p_this->mem_bmp.get());
				p_this->draw_memory();
				al_set_target_bitmap(al_get_backbuffer(p_this->curr_display));
				p_this->update_req = false;
			}else{
				this_thread::sleep_for(chrono::milliseconds(5));
			}
		}
	};

	void params_guard(){
		if(curr_display != al_get_current_display()) goto reattach;
		if(Nx != getNx() || Ny != getNy()) goto reattach;
		if(cx != al_get_display_width(curr_display) || cy != al_get_display_height(curr_display)) goto reattach;
		return;

		reattach:
		detach();
		attach();
	};

private:
	tBitmapPtr mem_bmp = nullptr;
	tBitmapPtr video_bmp = nullptr;
	ALLEGRO_DISPLAY* curr_display = nullptr;

	int Nx = 1, Ny = 1, cx = 1, cy = 1;
	double aspect = 1;
	int dx = 0, dy = 0;

	tThreadPtr mem_update_thread = nullptr;
	atomic_bool update_req;
	atomic_bool terminate;
};



#endif /* SRC_LIB_VISUAL_RENDERER_HPP_ */
