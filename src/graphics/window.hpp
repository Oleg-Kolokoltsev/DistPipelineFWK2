/*
 * window.h
 *
 *  Created on: Jan 13, 2016
 *      Author: Dr. Yevgeniy Kolokoltsev
 */

#ifndef WINDOW_H_
#define WINDOW_H_

#include <thread>
#include <memory>
#include <mutex>
#include <functional>
#include <stdexcept>
#include <chrono>
#include <atomic>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_memfile.h>

#include "renderer.hpp"

using namespace std;

class Window {
public:
	using tWindowPtr = shared_ptr<Window>;
	using tThreadPtr = unique_ptr<thread>;
	using tRendererPtr = unique_ptr<IRenderer>;
	using tDisplayPtr = unique_ptr<ALLEGRO_DISPLAY,function<void(ALLEGRO_DISPLAY*)>>;
	using tEventQueuePtr = unique_ptr<ALLEGRO_EVENT_QUEUE,function<void(ALLEGRO_EVENT_QUEUE*)>>;

	Window(tRendererPtr&& __renderer) : renderer{move(__renderer)}{
		running = false;
		terminate = false;

		if (!al_init())
			throw runtime_error("failed to initialize allegro");

		if(!al_install_mouse())
			throw runtime_error("failed to install mouse");

		if(!al_install_keyboard())
			throw runtime_error("failed to install keyboard");

		if(!al_init_primitives_addon())
			throw runtime_error("failed to initialize allegro primitives addon");

		if(!al_init_image_addon())
			throw runtime_error("failed to initialize allegro image addon");
	};

	void create_window(size_t width = 0, size_t height = 0){
		if(width == 0 || height == 0){
			width = renderer->getNx();
			height = renderer->getNy();
		}
		running = true;
		win_msg_proc.reset(new thread(Window::run, width, height, this));
	}

	void close(){
		terminate = true;
		if(win_msg_proc && win_msg_proc->joinable()){
			win_msg_proc->join();
		}
		win_msg_proc = nullptr;
	}

	bool is_running(){return running;};
	virtual ~Window(){renderer->detach(); close();};

private:

	static void run(size_t width, size_t height, Window* w){

		w->initialize(width,height);
		w->start_msg_queue();
		w->cleanup();

		w->running = false;
	};

	void initialize(size_t width, size_t height){
		display = tDisplayPtr(al_create_display(width,height),[=](ALLEGRO_DISPLAY* ptr){al_destroy_display(ptr);});
		if(!display) throw runtime_error("failed to create display");

		event_queue = tEventQueuePtr(al_create_event_queue(),[=](ALLEGRO_EVENT_QUEUE* ptr){al_destroy_event_queue(ptr);});
		if(!event_queue) throw runtime_error("failed to create event_queue");

		renderer->attach();
	};

	void start_msg_queue(){
		al_register_event_source(event_queue.get(), al_get_display_event_source(display.get()));
		al_register_event_source(event_queue.get(), al_get_mouse_event_source());
		al_register_event_source(event_queue.get(), al_get_keyboard_event_source());
		ALLEGRO_EVENT ev;

		while (1) {
			if(terminate) goto stop;
			while(al_get_next_event(event_queue.get(), &ev)){

				if(ev.any.source == al_get_mouse_event_source()){
					if(ev.mouse.display != display.get()) continue;
				}else if(ev.any.source == al_get_keyboard_event_source()){
					if(ev.keyboard.display != display.get()) continue;
				}

				if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) goto stop;
				renderer->msg_proc(ev);
			}

			renderer->render();
			this_thread::sleep_for(chrono::milliseconds(40));

		}

		stop:
		al_unregister_event_source(event_queue.get(),al_get_display_event_source(display.get()));
	};

	void cleanup(){
		renderer->detach();
		event_queue = nullptr;
		display = nullptr;
	}

	tDisplayPtr display;
	tEventQueuePtr event_queue;

	tThreadPtr win_msg_proc;
	tRendererPtr renderer;

	atomic_bool terminate;
	atomic_bool running;
};

#endif /* WINDOW_H_ */
