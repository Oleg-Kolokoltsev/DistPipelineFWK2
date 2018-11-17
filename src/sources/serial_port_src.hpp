//
// Created by morrigan on 2/7/18.
//

#ifndef DISTPIPELINEFWK_SERIAL_H
#define DISTPIPELINEFWK_SERIAL_H

#include <string>
#include <thread>
#include <mutex>
#include <list>
#include <array>
#include <vector>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "base_filter.hpp"

struct SerialOutPkt : public BaseMessage{
    // empty constructor
    SerialOutPkt(){}

    // copy constructor
    SerialOutPkt(const SerialOutPkt& pkt) : BaseMessage(pkt), block{pkt.block} {}

    // This client works in binary mode, each USB packet is a data block, and an information
    // shell be decoded afterwards.
    std::vector<char> block;
};


template<typename tIn, int MaxPktSize = 1024>
class SerialPortSRC : public BaseFilter<tIn, SerialOutPkt>{

protected:
    using tPtrSerialPort = boost::shared_ptr<boost::asio::serial_port>;
    using tErrCode = boost::system::error_code;
    using tBinBlock = std::array<char, MaxPktSize>;

public:
    using tBase = BaseFilter<tIn, SerialOutPkt>;
    using tPtrIn = typename tBase::tPtrIn;
    using tPtrOut = typename tBase::tPtrOut;

    SerialPortSRC(std::string dev_name) :
            tBase(nullptr, QUEUE_POLICY::DROP, dev_name), dev_name{dev_name} {}

protected:
    virtual void main_loop(){
        // asio service and it's thread live in this main only
        boost::asio::io_service io_service;

        // try to open serial port
        tErrCode ec;
        port = tPtrSerialPort(new boost::asio::serial_port(io_service));
        port->open(dev_name.c_str(), ec);
        if (ec) {
            print_err("port->open() failed, ", ec);
            return;
        }

        // configure the port if necessary
        // port->set_option( boost::asio::serial_port_base::baud_rate(9600) );
        // ... any other options

        // notify io_service that it has some work so it won't close it's run() immediately
        // at slow packet rates
        auto work = new boost::asio::io_service::work(io_service);

        // start the service and the acquisition loop
        std::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
        async_read_some();

        // main loop of this source thread
        while(1){

            //wait for the message
            tPtrIn curr_in = this->pull_msg(true);

            // process stop message
            if(curr_in && curr_in->cmd == MSG_CMD::STOP) break;
            if(!tBase::process_usr_msg(move(curr_in))){
                std::cerr << this->name << " warning: process_usr_msg failed" << std::endl;
            }

            asio_async_mtx.lock();
            while(!blocks.empty()){
                if(this->next){
                    std::shared_ptr<SerialOutPkt> pkt(new SerialOutPkt);
                    pkt->block = std::move(blocks.front());
                    this->next->put(move(pkt), this->uid, this->pol);
                }
                blocks.pop_front();
            }
            asio_async_mtx.unlock();
        }

        // close port and kill asio thread
        asio_async_mtx.lock();
        if (port && port->is_open()) {
            port->cancel();
            port->close();
            port.reset();
        }
        asio_async_mtx.unlock();

        delete work;
        io_service.stop();
        io_service.reset();
        if(t.joinable()) t.join();
    }

protected:

    void async_read_some(){
        if (!port || !port->is_open()) return;
        using namespace boost::asio;
        port->async_read_some(
                buffer(read_buf_raw.data(), std::tuple_size<tBinBlock>::value),
                boost::bind(
                        &on_receive,
                        placeholders::error,
                        placeholders::bytes_transferred,
                        this));
    }

    static void on_receive(const tErrCode& ec, size_t bytes_transferred, SerialPortSRC* p_src){
        p_src->asio_async_mtx.lock();

        if (p_src->port && p_src->port->is_open()) {
            if (ec) {
                    p_src->print_err("on_receive failed, ", ec);
            } else {
                /*
                 * It is a good practice to keep MaxPktSize about 1.3 times greater than
                 * the real maximum packet size/ In this case no packets will be
                 * divided into separate data blocks that is strongly undesirable
                 * for subsequent processing. Therefore, we pt a warning here:
                 */
                if(bytes_transferred == MaxPktSize)
                    std::cerr << p_src->dev_name
                              << " Warning: MaxPktSize (" << MaxPktSize
                              << ") reached, the packet will spread into more than one block."
                              << std::endl;

                std::vector<char> b(bytes_transferred);
                memcpy(b.data(), p_src->read_buf_raw.data(), bytes_transferred);
                p_src->blocks.push_back(std::move(b));

                /*
                 * This message is sent to make one rotation of the main cycle.
                 * During this rotation the p_src->blocks will be emptied. It is
                 * dangerous to use WAIT policy here, because if it happens during
                 * main loop, there can appear a dead-lock on the asio_async_mtx.
                 * However, it is completely safe to use DROP policy, because
                 * if one ACQUIRE is occasionally dropped, during the next one
                 * all data will be pulled from the p_src->blocks anyway.
                 */
                p_src->put(MSG_CMD::ACQUIRE, p_src->uid, QUEUE_POLICY::DROP);

                // the only problem here, is that if the next node is slow and is used
                // with WAIT policy, the p_src->blocks can increase until the end of memory
                // so we make a warning here:

                if(p_src->blocks.size() > 1000)
                    std::cerr << p_src->dev_name
                              << "Warning: source has too many packets (1000)" << std::endl;
            }
        }

        p_src->asio_async_mtx.unlock();

        p_src->async_read_some();
    }

    void print_err(std::string msg, const tErrCode& ec){
        std::cerr << msg << (msg.size() ? " " : "")
                  << dev_name << ": " << ec.message() << "( code = " << ec.value() << " )" << std::endl;

        //todo:
        //Works fine for Arduino Uno
        //See http://docs.ros.org/lunar/api/libmavconn/html/serial_8cpp_source.html for automatic solution
        if(ec.value() == boost::asio::error::eof)
        std::cerr << "(workaround) say: \"stty -F /dev/ttyACM0 raw\"" << std::endl;
    }


protected:
    // serial port data
    std::string dev_name;
    size_t baud_rate;
    tPtrSerialPort port;

    // needed to sync data access from additional asynchronous acquisition thread (asio)
    std::mutex asio_async_mtx;

    // plain buffer that is filled with data by asio
    tBinBlock read_buf_raw;

    // internal data queue
    // todo: use std::deque as in plotscope
    std::list<std::vector<char>> blocks;
};

#endif //DISTPIPELINEFWK_SERIAL_H
