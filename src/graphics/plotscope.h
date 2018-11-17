//
// Created by morrigan on 8/22/18.
//

#ifndef DISTPIPELINEFWK_PLOTSCOPE_H
#define DISTPIPELINEFWK_PLOTSCOPE_H

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <deque>
#include <mutex>

#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_memfile.h>

#include <gsl/gsl_matrix.h>

#include "renderer.hpp"
#include "genoscdevice.hpp"
#include "psmsgnsig_txt.h"

class PlotScope : public Bitmap2DRenderer{

    using tPtrSrc = std::shared_ptr<GenOscDevice<PSIMsg>>;
    using tErrCode = boost::system::error_code;
    using tBuf = std::vector<unsigned char>;
    using tAsyncPipe = boost::process::async_pipe;
    using tPtrAsyncPipe = std::shared_ptr<tAsyncPipe>;
    using tFHandleRead = std::function<void(const tErrCode&, std::size_t, tPtrAsyncPipe, PlotScope*)>;
    using tIOSrv = boost::asio::io_service;
    using tPtrIOSrv = std::shared_ptr<tIOSrv>;
    using tChild = boost::process::child;
    using tPtrChild = std::shared_ptr<tChild>;
    using tThread = std::thread;
    using tPtrThread = std::shared_ptr<tThread>;

public:

    //todo: make something with resolution!
    PlotScope(tPtrSrc, size_t);
    ~PlotScope();

    size_t& getNx(){return Nx;};
    size_t& getNy(){return Ny;};

    void push_gp(std::string cmd){push_gp_in(in.get(), cmd);}

protected:
    virtual void draw_memory();
    virtual void msg_proc(const ALLEGRO_EVENT& ev);

private:
    void is_err(const tErrCode& ec);
    void read_some_wrapper(tFHandleRead, tPtrAsyncPipe, tBuf&, PlotScope*);
    static void push_gp_in(tAsyncPipe*, std::string&);
    static void handle_read_stderr(const tErrCode&, std::size_t, tPtrAsyncPipe, PlotScope*);
    static void handle_read_stdout(const tErrCode&, std::size_t, tPtrAsyncPipe, PlotScope*);

private:
    //*** SREEN
    tPtrSrc src;
    size_t Nx; // screen width (x10)
    size_t Ny; // screen height (x8)

    //*** ASIO
    tPtrIOSrv ios; // IO service
    tPtrThread ioth; // thread for IO service
    tPtrAsyncPipe out, err, in; // standard IO pipes (stdout, stderr, stdin)
    tPtrChild gp; // gnuplot child process
    atomic<bool> stopped; // required to gracefully terminate read_some cycle
    tBuf err_buf; // plain buffer for ASIO
    tBuf out_buf; // plain buffer for ASIO

    //*** PNG data
    //todo: use it for slow data processing
    //atomic<bool> gp_received; // shows if a gnuplot already has prepared a new image in curr_png buffer
    tBuf curr_png; // if not empty - contain a complete PNG image
    std::mutex curr_png_mtx;
    std::deque<tBuf::value_type> part_png; // part of the image that gnuplot is currently sending via std_out (deque is for erase)

    //*** slising of gnuplot stdout onto PNG files
    const tBuf PNG_HEAD = {137, 80, 78, 71, 13, 10, 26, 10};
    const tBuf PNG_FOOT = {'I','E','N','D'};
    bool head_found = false;
};



#endif //DISTPIPELINEFWK_PLOTSCOPE_H
