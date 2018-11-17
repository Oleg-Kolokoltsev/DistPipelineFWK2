//
// Created by morrigan on 8/30/18.
//

#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

#include "plotscope.h"

//nice answer about centering 0 frequency
//https://mathematica.stackexchange.com/questions/33574/whats-the-correct-way-to-shift-zero-frequency-to-the-center-of-a-fourier-transf

using namespace std;

PlotScope::PlotScope(tPtrSrc src, size_t resolution = 40) : Bitmap2DRenderer(),
src{src}, Nx{10*resolution}, Ny{8*resolution},
err_buf(4096), out_buf(4096), stopped{false} {

    namespace bp = boost::process;

    auto path = bp::search_path("gnuplot");
    if(path.string().empty()) throw runtime_error("failed to find gnuplot executable in system $PATH");
    cout << "Found gnuplot: " << path.string() << endl;

    stringstream ss;
    ss << "set terminal png size " << Nx << "," << Ny << endl;
    string plt_header = ss.str();

    ios = tPtrIOSrv(new tIOSrv,
            [] (tIOSrv* p_ios){
        if(!p_ios->stopped()) p_ios->stop();
        delete p_ios;
    });

    out = tPtrAsyncPipe(new tAsyncPipe(*ios),
            [] (tAsyncPipe* p_out){
        if(p_out->is_open()) p_out->close();
        delete p_out;
    });

    err = tPtrAsyncPipe(new tAsyncPipe(*ios),
            [] (tAsyncPipe* p_err){
        if(p_err->is_open()) p_err->close();
        delete p_err;
    });

    in = tPtrAsyncPipe(new tAsyncPipe(*ios),
            [] (tAsyncPipe* p_in){
        if(p_in->is_open()) {
            auto cmd = string("quit\n");
            push_gp_in(p_in, cmd);
            p_in->close();
        }
        delete p_in;
    });

    gp = tPtrChild(new tChild(path, bp::std_out > *out, bp::std_err > *err, bp::std_in < *in),
            [](tChild* p_gp){
        if(p_gp->joinable()){
            p_gp->wait_for(chrono::seconds(3));
            p_gp->join();
        }
        delete p_gp;
    });

    ioth = tPtrThread(new tThread(boost::bind(&tIOSrv::run, ios)),
            [] (tThread* p_ioth){
        if(p_ioth->joinable()) p_ioth->join();
        delete p_ioth;
    });

    read_some_wrapper(handle_read_stdout, out, out_buf, this);
    read_some_wrapper(handle_read_stderr, err, err_buf, this);

    push_gp_in(in.get(), plt_header);
}

PlotScope::~PlotScope(){
    // ensure destruction order
    stopped = true;
    in.reset();
    gp.reset();
    ioth.reset();
    out.reset();
    err.reset();
    ios.reset();
};

// ****** ASIO PIPES ******

void PlotScope::push_gp_in(tAsyncPipe* p_in, std::string& cmd){
    size_t written = 0, len = std::strlen(cmd.c_str());
    do{
        written += p_in->write_some(boost::asio::buffer(cmd.c_str(), len));
    }while(written < len);
}

void
PlotScope::read_some_wrapper(tFHandleRead handler, tPtrAsyncPipe pipe, tBuf& buf, PlotScope* ps){
    if(pipe && pipe->is_open() && !ps->stopped)
        pipe->async_read_some(boost::asio::buffer(buf),
                              boost::bind(
                                      handler,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred,
                                      pipe,
                                      this));
}

void
PlotScope::handle_read_stderr(const tErrCode& ec, std::size_t bytes_transferred, tPtrAsyncPipe pipe, PlotScope* ps){
    if(ps->stopped) return;
    if(ec){
        ps->is_err(ec);
        return;
    }

    if(bytes_transferred > 0)
        cerr.write(static_cast<char*>((void*)ps->err_buf.data()), bytes_transferred);

    ps->read_some_wrapper(handle_read_stderr, pipe, ps->err_buf, ps);
}

void
PlotScope::handle_read_stdout(const tErrCode& ec, std::size_t bytes_transferred, tPtrAsyncPipe pipe, PlotScope* ps){
    if(ps->stopped) return;
    if(ec){
        ps->is_err(ec);
        return;
    }

    //Data format: PNG_HEAD ... PNG_FOOT + 4 byte CRC

    if(bytes_transferred > 0) {
        auto& part_png = ps->part_png;

        auto prev_sz = part_png.size();
        part_png.insert(part_png.end(), ps->out_buf.begin(), ps->out_buf.begin() + bytes_transferred);

        auto roll = prev_sz > 8 ? 7 : prev_sz;

        if(!ps->head_found){
            auto head_it = search(part_png.begin() + prev_sz - roll, part_png.end(),
                    ps->PNG_HEAD.begin(), ps->PNG_HEAD.end());

            if(head_it != part_png.end()){
                if(head_it != part_png.begin()) {
                    cerr << endl << "DEBUG: ERASED RUBUSH 1 (shell never happen)" << endl;
                    part_png.erase(part_png.begin(), head_it);
                }
                ps->head_found = true;
            }
        }else{
            auto foot_it = search(part_png.begin() + prev_sz - roll, part_png.end(),
                              ps->PNG_FOOT.begin(), ps->PNG_FOOT.end());

            if(foot_it != part_png.end() && distance(foot_it, part_png.end()) >= 8){

                // search for any head duplicates after the first one
                auto head_it = search(part_png.begin() + 1, part_png.end(),
                                      ps->PNG_HEAD.begin(), ps->PNG_HEAD.end());

                while(head_it != part_png.end() && head_it < foot_it){
                    cerr << endl << "DEBUG: ERASED RUBUSH 2 (shell never happen)" << endl;
                    // todo: erase is [part_png.begin(), head_it) - is it correctly used here everywhere?
                    part_png.erase(part_png.begin(), head_it);
                    head_it = search(part_png.begin() + 1, part_png.end(),
                                     ps->PNG_HEAD.begin(), ps->PNG_HEAD.end());
                }

                // valid PNG block found and localized
                ps->head_found = false;

                ps->curr_png_mtx.lock();
                ps->curr_png.resize(distance(part_png.begin(), foot_it + 8));
                ps->curr_png.insert(ps->curr_png.begin(), part_png.begin(), foot_it + 8);
                ps->curr_png_mtx.unlock();


                part_png.erase(part_png.begin(), foot_it + 8);

                //using namespace std::chrono;
                //auto ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
                //cout << ms << endl;
            }
        }

        //todo: remove comments
        //cout << endl << "CLEAN PNG DATA" << endl;
        //cout.write(static_cast<char *>((void *) ps->curr_png.data()), ps->curr_png.size());
        //cout << ps->curr_png.size() << endl;
        //cout << endl << "END OF FILE" << endl;
        //cout << ps->curr_png.size() << endl;
    }

    ps->read_some_wrapper(handle_read_stdout, pipe, ps->out_buf, ps);
}

void
PlotScope::is_err(const tErrCode& ec){
        std::cerr << "PlotScope: " << ec.message() << "( code = " << ec.value() << " )" << std::endl;
}

// ****** GRAPHICS ******

void
PlotScope::draw_memory(){

    // draw current PNG image
    curr_png_mtx.lock();
    if(!curr_png.empty()) {
        ALLEGRO_FILE *memfile = al_open_memfile(curr_png.data(), curr_png.size()*sizeof(tBuf::value_type), "rb");
        ALLEGRO_BITMAP *bitmap = al_load_bitmap_f(memfile, ".png");
        //ALLEGRO_BITMAP* bitmap = al_load_bitmap("/home/morrigan/image.png");
        al_draw_bitmap(bitmap, 0, 0, 0);
        al_destroy_bitmap(bitmap);
        al_fclose(memfile);
    }
    curr_png_mtx.unlock();

    // send next command to gnuplot (todo: can overflow stdin of gnuplot if data processing is too slow)
    auto frame = src->get_frame();
    std::string cmd;
    frame->get_data(cmd);

    if (in) {
        push_gp_in(in.get(), cmd);
        //std::cout << cmd << std::endl;
    }
}

void PlotScope::msg_proc(const ALLEGRO_EVENT& ev){

}





