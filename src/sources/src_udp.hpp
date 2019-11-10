//
// Created by morrigan on 11/9/19.
//

#ifndef DISTPIPELINEFWK_SRC_UDP_H
#define DISTPIPELINEFWK_SRC_UDP_H

#include <thread>
#include <mutex>
#include <memory>
#include <regex>
#include <deque>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include "base_source.hpp"



struct UDPOutPkt : public BaseMessage{
    // empty constructor
    UDPOutPkt(){}

    // copy constructor
    UDPOutPkt(const UDPOutPkt& pkt) : BaseMessage(pkt), block{pkt.block} {}

    // This client works in binary mode, each UDP packet is a data block, and an information
    // shell be decoded afterwards.
    std::vector<char> block;
};

template<int MaxPktSize = 1024>
class UDPSource : public BaseSource<UDPOutPkt>{

protected:
    using tUDP = boost::asio::ip::udp;
    using tEndpoint = tUDP::endpoint;
    using tPtrSocket = std::shared_ptr<tUDP::socket>;
    using tErrCode = boost::system::error_code;
    using tIP = boost::asio::ip::address;
    using tBinBlock = std::array<char, MaxPktSize>;

public:
    using tBase = BaseSource<UDPOutPkt>;
    using tPtrIn = typename tBase::tPtrIn;
    using tPtrOut = typename tBase::tPtrOut;

    UDPSource(std::string listen, std::string dev_name = "UDPSource") :
    tBase(nullptr, QUEUE_POLICY::DROP, dev_name), listen{listen} {}

protected:
    virtual void main_loop(){

        // read IP and port from the 'listen' string
        auto pos = listen.find(':');
        std::string str_ip = listen.substr(0, pos);
        std::string str_port = listen.substr(pos+1, listen.length());

        tIP ip;
        int port;

        try {
            ip = tIP::from_string(str_ip);
            port = boost::lexical_cast<int>(str_port);
        }catch(...){
            std::cerr << tBase::name << ": Can't parse: " << listen << std::endl;
            std::cerr << tBase::name << ": Example: 192.168.1.50:1234" << listen << std::endl;
        }

        // create endpoints and open a listening socket
        tEndpoint local_endpoint = tUDP::endpoint(ip, port);

        boost::asio::io_service io_service;

        try {
            socket = tPtrSocket(new tPtrSocket::element_type(io_service));
            socket->open(tUDP::v4());
            socket->bind(local_endpoint);
            std::cout << tBase::name << ": listening to " << local_endpoint << std::endl;
        }catch(...){
            std::cerr << tBase::name << ": can't open UDP port " << port << std::endl;
        }

        // notify io_service that it has some work so it won't close it's run() immediately
        // at slow packet rates
        auto work = new boost::asio::io_service::work(io_service);

        // start the service and the acquisition loop
        std::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
        async_read_some();

        while(1){
            //wait for the message
            tPtrIn curr_in = this->pull_msg(true);

            // process stop message
            if(curr_in && curr_in->cmd == MSG_CMD::STOP) break;

            asio_async_mtx.lock();
            while(!blocks.empty()){
                if(this->next){
                    std::shared_ptr<UDPOutPkt> pkt(new UDPOutPkt);
                    pkt->block = std::move(blocks.front());
                    this->next->put(move(pkt), this->uid, this->pol);
                }
                blocks.pop_front();
            }
            asio_async_mtx.unlock();
        }

        // close port and kill asio thread
        asio_async_mtx.lock();
        if (socket && socket->is_open()) {
            socket->cancel();
            socket->close();
        }
        asio_async_mtx.unlock();

        delete work;
        io_service.stop();
        io_service.reset();
        if(t.joinable()) t.join();
    }

protected:

    void async_read_some(){
        if (!socket || !socket->is_open()) return;
        using namespace boost::asio;

        socket->async_receive_from(
                buffer(read_buf_raw.data(), MaxPktSize),
                remote_endpoint,
                boost::bind(
                        &on_receive,
                        placeholders::error,
                        placeholders::bytes_transferred,
                        this));
    }

    static void on_receive(const tErrCode& ec, size_t bytes_transferred, UDPSource* p_src){
        p_src->asio_async_mtx.lock();

        if (p_src->socket || p_src->socket->is_open()) {
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
                    std::cerr << p_src->name
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
                    std::cerr << p_src->name
                              << "Warning: source has too many packets (1000)" << std::endl;
            }
        }

        p_src->asio_async_mtx.unlock();
        p_src->async_read_some();
    }

    void print_err(std::string msg, const tErrCode& ec){
        // ignore "Operation canceled error"
        // it is used before socket close
        if(ec.value() == 125) return;

        // print an error
        std::cerr << msg << (msg.size() ? " " : "")
                  << tBase::name << ": " << ec.message() << "( code = " << ec.value() << " )" << std::endl;
    }

protected:
    // string that defines a local IP and port to be listened
    // (different interfaces can have different IP's)
    // example: 192.168.1.10:1234
    std::string listen;

    // local UDP socket to be listened
    tPtrSocket socket;

    // remote andpoint structure for internal asio usage
    tEndpoint remote_endpoint;

    // plain buffer that is filled with data by asio
    tBinBlock read_buf_raw;

    // needed to sync data access from additional asynchronous
    // acquisition thread (asio)
    std::mutex asio_async_mtx;

    // internal data queue
    std::deque<std::vector<char>> blocks;
};

#endif //DISTPIPELINEFWK_SRC_UDP_H
