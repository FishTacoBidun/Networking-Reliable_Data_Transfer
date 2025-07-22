#pragma once

#include <string>
#include <sstream>
#include <random>
#include <chrono>

using std::string;
using std::ostringstream;
using std::random_device;
using std::default_random_engine;
using std::chrono::system_clock;

/**
 * Segment class
 * 
 * Description:
 *   The segment is a segment of data to be transferred on a cummunication channel.
 * 
 * 
 * Notes:
 *   This file is not to be changed.
 * 
 * 
 **/
class Segment {
    //private by default, but I like to be explicit!
    private:
        int seqnum;
        int acknum;
        string payload;
        int checksum;
        int startIteration;
        int startDelayIteration;

    public:
        Segment(){
            this->seqnum = -1;
            this->acknum = -1;
            this->payload = "";
            this->checksum = 0;
            this->startIteration = 0;
            this->startDelayIteration = 0;
        }

        void setData(int seq, string data){
            this->seqnum = seq;
            this->acknum = -1;
            this->payload = data;
            this->checksum = 0;

            string str = to_string();
            this->checksum = calc_checksum(str);
        }

        void setAck(int ack){
            this->seqnum = -1;
            this->acknum = ack;
            this->payload = "";
            this->checksum = 0;

            string str = to_string();
            this->checksum = calc_checksum(str);
        }

        int getAck(){
            return this->acknum;
        }

        void setStartIteration(int iteration){
            this->startIteration = iteration;
        }

        int getStartIteration(){
            return this->startIteration;
        }

        void setStartDelayIteration(int iteration){
            this->startDelayIteration = iteration;
        }

        int getStartDelayIteration(){
            return this->startDelayIteration;
        }

        string to_string(){
            //Using string stream instead of a bunch of manual casting.
            ostringstream oss;
            oss << "seq: " << this->seqnum << ", ack: " << this->acknum << ", data: " << this->payload;

            return oss.str();
        }

        bool checkChecksum(){
            int cs = calc_checksum(to_string());

            return cs == this->checksum;
        }

        //Sum and return the value of a string's characters as unicode values.
        int calc_checksum(string str){
            //Naive approach, could use reduce instead with parallel executor, C++17, numeric header.
            int cs = 0;
            for(int i = 0; i < str.length(); ++i){
                cs += str[i];
            }

            return cs;
        }

        void createChecksumError(){
            if(this->payload == ""){
                return;
            }

            //Seed random number generator.
            default_random_engine rng(random_device{}());
            //Select a random value, but make sure it is in range of the length of our payload.
            int char_pos = rng() % this->payload.length();

            this->payload[char_pos] = 'X';
        }
};

/**
 * Segment helper non-member functions
 **/
inline bool operator==(const Segment& lhs, const Segment& rhs){ return lhs == rhs; }
inline bool operator!=(const Segment& lhs, const Segment& rhs){return !operator==(lhs,rhs);}
