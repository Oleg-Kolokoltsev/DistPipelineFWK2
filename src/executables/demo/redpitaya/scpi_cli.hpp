//
// Created by morrigan on 1/31/17.
//

#ifndef COMPPHYSFWK_SCPI_CLI_HPP
#define COMPPHYSFWK_SCPI_CLI_HPP

#include <iostream>
#include <vector>
#include <chrono>
#include <array>
#include <string>

#include <boost/asio.hpp>

#include "base_source.hpp"

inline void sleep_ms(int ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
};

struct BIN_PKT : public BaseMessage{

    //empty constructor
    BIN_PKT(){}

    //copy constructor
    BIN_PKT(const BIN_PKT& pkt) : BaseMessage(pkt){
        bin_frame = pkt.bin_frame;
        t_adq = pkt.t_adq;
        measure_time = pkt.measure_time;
    }

    std::vector<char> bin_frame; //data
    double t_adq; //measured acquisition time
    double measure_time; //full time of data array (x-axis)
};

class SCPIClient : public BaseSource<BIN_PKT>{
public:
    using tBase = BaseSource<BIN_PKT>;
    using tPtrIn = tBase::tPtrIn;

    //todo: protected constructor - there is a node factory!
    SCPIClient(std::string ip_str, std::string port) : ip_str{ip_str}, port{port},
                                              tBase(nullptr/*main is overridden, so no matter*/,
                                                    QUEUE_POLICY::DROP/*it is important to keep request queue on server,
                                              *so better drop binary data in the case when next filter is slower than
                                              *acquisition speed*/,
                                                    "SCPIClient"){}
protected:
    virtual void main_loop(){

        boost::asio::ip::tcp::iostream s_tcp; //tcp channel
        std::string line; //string line for ASCII I/O
        std::chrono::steady_clock::time_point t0, t1; //measure transfer rate
        char c; //byte to read from input stream
        std::vector<char> ch_n_pts;//number of binary points to read

        //TODO: Find IP by known MAC, but test with redpitaya!!!
        //Connect to server
        s_tcp.connect(ip_str,port);

        std::cout << "connected to " << ip_str << std::endl;

        //Configure acquisition mode

        s_tcp << "ACQ:RST;" << "\r\n";
        s_tcp << "ACQ:DATA:FORMAT BIN" << "\r\n";
        //this is always ON by default
        s_tcp << "ACQ:AVG OFF" << "\r\n";

        //redpitaya has 125 MHz ADC, so full buffer (16384 pts) is 131.072us
        //the decimation factor multiply a whole time period, so that
        const double measure_time1 = 0.000131072;
        double DEC = 64;
        s_tcp << "ACQ:DEC "<< (int)DEC << "\r\n";/* 1(131mks)  8(1ms)  64(8.4ms)  1024(134ms)  8192(1s)  65536(8.6s) */


        //s_tcp << "ACQ:AVG?" << "\r\n";
        //s_tcp >> line;
        //cout << "Is acquire averaged? " << line << endl;

        //after trigger we have to wait until this number of points to be captured
        //so far we would know that the buffer was completely renewed and no delay after
        //ACQ:START is needed
        s_tcp << "ACQ:TRIG:DLY 8192"  << "\r\n";
        s_tcp << "ACQ:TRIG:DLY?"  << "\r\n";
        s_tcp >> line;
        std::cout << "Trigger delay (samples) " << line << std::endl;
        s_tcp << ":ACQ:TRIG:LEV 500" << "\r\n";

        int requests_pending = 0;
        bool stop = false;
        while(1){

            //check for terminate condition (fast, unblocking pull)
            tPtrIn curr_in = pull_msg(false);
            if(curr_in && curr_in->cmd == MSG_CMD::STOP) stop = true;

            //start measuring data transfer rate
            t0 = std::chrono::steady_clock::now();

            //request for single data block
            if(!stop){

                s_tcp << "ACQ:START" << "\r\n";
                s_tcp << "ACQ:TRIG EXT_PE" << "\r\n"; //EXT_NE

                while(1){
                    s_tcp << "ACQ:TRIG:STAT?" << "\r\n";
                    s_tcp >> line;
                    std::cout << line << "; ";
                    if (line.find("TD") != std::string::npos){ std::cout << std::endl; break; }
                }

                s_tcp << "ACQ:SOUR1:DATA?" << "\r\n";

                requests_pending++;
                //DO NOT keep small request queue on the server (speed boost x2) - old code
                if(requests_pending < 1) continue;
            }else if(requests_pending == 0){
                //get out from acquisition loop only after input queue
                //was read completely
                break;
            }

            //read byte by byte until data start delimiter
            do{ s_tcp.read(&c,1); } while(c != '#');

            //read number of chars describing data length
            s_tcp.read(&c,1);
            auto l = stoi(std::string(1,c));

            //read data length and convert it to numeric type
            ch_n_pts.clear();
            ch_n_pts.resize(l+1);
            s_tcp.read(ch_n_pts.data(),l);
            ch_n_pts.at(l) = 0;
            //TODO: check size is correct (multiple of 4, less than buffer), apply asserts
            auto dl = stoi(std::string(ch_n_pts.data()));


            //read points
            std::shared_ptr<BIN_PKT> pkt(new BIN_PKT);
            pkt->bin_frame.resize(dl);
            s_tcp.read(static_cast<char*>((void*)pkt->bin_frame.data()),dl);
            requests_pending--;

            //fill acquisition time and time scale;
            t1 = std::chrono::steady_clock::now();
            pkt->t_adq = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
            pkt->measure_time = measure_time1*DEC;

            //send data to the next processing node
            if(next){
                next->put(move(pkt), this->uid, pol);
            }else{
                pkt.reset();
                std::cerr << name << " no destination, data lost" << std::endl;
            }
        }  // end_while

        s_tcp.flush();
        std::cout << "Reset server, close connection" << std::endl;
        s_tcp << "ACQ:RST" << "\r\n";
        s_tcp.flush();

        s_tcp.close();
    }
private:
    std::string ip_str;
    std::string port;
};

#endif //COMPPHYSFWK_SCPI_CLI_HPP
