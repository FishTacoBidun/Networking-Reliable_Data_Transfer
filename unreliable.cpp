#pragma once

#include <vector>
#include <iostream> //for quick debug purposes
#include <random>
#include <algorithm> //for reverse

//really should split definitions from implementation, but being lazy right now...
#include "Segment.cpp"

using std::vector;
using std::cout;
using std::endl;
using std::random_device;
using std::mt19937;
using std::uniform_real_distribution;
using std::reverse;
using std::remove;

/**
 * ToDo:
 *  Test debug statements to make sure they look alright!
 **/



/**
 * UnreliableChannel
 * 
 * Description:
 *   This class is meant to be more of a blackbox but you are allowed to see the implementation. You are not allowed to
 *   change anything in this file. There is also no need to base your algorithms on this particular implementation.
 * 
 * 
 * Notes: 
 *   This file is not to be changed.
 * 
 * 
 **/

class UnreliableChannel{
    //private by default, but I like to be explicit!
    private:
        //Note, these look like constants, but...
        double RATIO_DROPPED_PACKETS;
        double RATIO_DELAYED_PACKETS;
        double RATIO_DATA_ERROR_PACKETS;
        double RATIO_OUT_OF_ORDER_PACKETS;
        int ITERATIONS_TO_DELAY_PACKETS;

        vector<Segment> sendQueue;
        vector<Segment> receiveQueue;
        vector<Segment> delayedPackets;

        bool canDeliverOutOfOrder;
        bool canDropPackets;
        bool canDelayPackets;
        bool canHaveChecksumErrors;

        //stats
        int countTotalDataPackets;
        int countSentPackets;
        int countChecksumErrorPackets;
        int countDroppedPackets;
        int countDelayedPackets;
        int countOutOfOrderPackets;
        int countAckPackets;
        int currentIteration;

    public: 
        UnreliableChannel(bool canDeliverOutOfOrder_, bool canDropPackets_, bool canDelayPackets_, bool canHaveChecksumErrors_){
            this->RATIO_DROPPED_PACKETS = 0.1;
            this->RATIO_DELAYED_PACKETS = 0.1;
            this->RATIO_DATA_ERROR_PACKETS = 0.1;
            this->RATIO_OUT_OF_ORDER_PACKETS = 0.1;
            this->ITERATIONS_TO_DELAY_PACKETS = 5;

            this->canDeliverOutOfOrder = canDeliverOutOfOrder_;
            this->canDropPackets = canDropPackets_;
            this->canDelayPackets = canDelayPackets_;
            this->canHaveChecksumErrors = canHaveChecksumErrors_;

            this->countTotalDataPackets = 0;
            this->countSentPackets = 0;
            this->countChecksumErrorPackets = 0;
            this->countDroppedPackets = 0;
            this->countDelayedPackets = 0;
            this->countOutOfOrderPackets = 0;
            this->countAckPackets = 0;
            this->currentIteration = 0;
        }

        /* ************************************************************************************************************** */
        /* Getters                                                                                                        */
        /*                                                                                                                */
        /*                                                                                                                */
        /*                                                                                                                */
        /*                                                                                                                */
        /* ************************************************************************************************************** */
        int getCountTotalDataPackets() {
            return this->countTotalDataPackets;
        }

        int getCountSentPackets() {
            return this->countSentPackets;
        }

        int getCountChecksumErrorPackets() {
            return this->countChecksumErrorPackets;
        }

        int getCountDroppedPackets() {
            return this->countDroppedPackets;
        }

        int getCountDelayedPackets() {
            return this->countDelayedPackets;
        }

        int getCountOutOfOrderPackets() {
            return this->countOutOfOrderPackets;
        }

        int getCountAckPackets() {
            return this->countAckPackets;
        }

        int getCurrentIteration() {
            return this->currentIteration;
        }

        void send(Segment seg){
            this->sendQueue.push_back(seg);
        }

        vector<Segment> receive(){
            vector<Segment> new_list(receiveQueue);
            this->receiveQueue.clear();
            
            //debug statement
            cout << "UnreliableChannel length of receiveQueue: " << this->receiveQueue.size() << endl;

            return new_list;
        }

        void processData(){
            //debug!
            cout << "UnreliableChannel manage - len sendQueue: " << this->sendQueue.size() << endl;

            this->currentIteration++;

            if(this->sendQueue.size() == 0){
                return;
            }

            //Quick reference for random uniform real distribution: https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
            random_device rd;
            mt19937 rng(rd());
            uniform_real_distribution<> dis(0.0, 1.0);

            if(this->canDeliverOutOfOrder){
                double val = dis(rng);

                if(val < this->RATIO_OUT_OF_ORDER_PACKETS){
                    this->countOutOfOrderPackets++;
                    reverse(this->sendQueue.begin(), this->sendQueue.end());
                }
            }

            //add in delayed packets
            vector<Segment> noLongerDelayed;
            for(auto seg : this->delayedPackets){
                int numIterDelayed = this->currentIteration - seg.getStartDelayIteration();
                if(numIterDelayed >= this->ITERATIONS_TO_DELAY_PACKETS){
                    noLongerDelayed.push_back(seg);
                }
            }

            for(auto seg : noLongerDelayed){
                this->countSentPackets++;
                remove(this->delayedPackets.begin(), this->delayedPackets.end(), seg);
                this->receiveQueue.push_back(seg);
            }

            for(auto seg : this->sendQueue){
                //receiveQueue.push_back(seg);

                bool addToReceiveQueue = false;
                if(this->canDelayPackets){
                    double val = dis(rng);
                    if(val <= this->RATIO_DELAYED_PACKETS){
                        this->countDelayedPackets++;
                        seg.setStartDelayIteration(this->currentIteration);
                        this->delayedPackets.push_back(seg);
                        continue;
                    }
                }

                if(this->canDropPackets){
                    double val = dis(rng);
                    if(val <= this->RATIO_DROPPED_PACKETS){
                        this->countDroppedPackets++;
                    } else{
                        addToReceiveQueue = true;
                    }
                } else{
                    addToReceiveQueue = true;
                }

                if(addToReceiveQueue){
                    this->receiveQueue.push_back(seg);
                    this->countSentPackets++;
                }

                if(seg.getAck() == -1){
                    this->countTotalDataPackets++;

                    //only data packets can have checksum errors...
                    if(this->canHaveChecksumErrors){
                        double val = dis(rng);
                        if(val <= this->RATIO_DATA_ERROR_PACKETS){
                            seg.createChecksumError();
                            this->countChecksumErrorPackets++;
                        }
                    }
                }else {
                    this->countAckPackets++;
                }
                cout << "UnreliableChannel len receiveQueue: " << this->receiveQueue.size() << endl;
            }
            this->sendQueue.clear();
            cout << "UnreliableChannel manage - len receiveQueue: " << this->receiveQueue.size() << endl;
        } //end processData function
}; //end UnreliableChannel class
