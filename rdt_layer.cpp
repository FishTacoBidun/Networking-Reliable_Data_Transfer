#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>
//added libraries
#include <map>
#include <set>

#include "segment.cpp"
#include "unreliable.cpp"

using std::cout;
using std::endl;
using std::optional;
using std::string;
using std::vector;
//added usings
using std::map;
using std::set;
using std::min;

/**
 * RDTLayer
 * 
 * Description:
 *   The reliable data transfer (RDT) layer is used as a communication layer to resolve issues over an unreliable
 *   channel.
 * 
 * 
 * Notes:
 * This file is meant to be changed.
 * 
 * 
 **/
class RDTLayer{
    /**
     * Class Scope Variables
     * 
     * 
     * 
     **/
    //private by default, but I prefer being explicit!
    private:
        int DATA_LENGTH = 4;            //The length of the string data that will be sent per packet in characters.
        int FLOW_CONTROL_WIN_SIZE = 10;  //Receive window size for flow-control.
        int TIMEOUT = 5;

        /*Note: sendChannel and receiveChannel are set in the rdt_main...
                We are using the optional<UnreliableChannel> type to have behavior
                similar to using none that is then set to an object in Python.
                
                We could use pointers all over, but since we do not do any none or nullptr checking
                or do any pointer manipulation anyway...
        */
        optional<UnreliableChannel> sendChannel;
        optional<UnreliableChannel> receiveChannel;

        string dataToSend;
        string assembledData;

        int currentIteration;       //Use this for segment "timeouts"
        int countSegmentTimeouts; //use this to track how many segments actually timeout

        /************************************************************
         * Add items here as needed!                                *
         ************************************************************/
        
        //sending side
        int sendBase = 0;
        int nextSeqNum = 0;
        map<int, Segment> sentSegments;
        map<int, int> sendTimestamps;

        //receiving side
        int expectedSeqNum = 0;
        map<int, string> receiveBuffer;
        set<int> receivedSeqNums;

        bool isSender() 
        {
            return !dataToSend.empty();
        }

    public:
        RDTLayer(){
            this->dataToSend = "";
            this->currentIteration = 0;
            this->countSegmentTimeouts = 0;

            /************************************************************
             * Add items here as needed!                                *
             ************************************************************/
        }

        /**
         * setSendChannel()
         * 
         * Description:
         *   Called by main to set the unreliable sending lower-layer channel
         * 
         * 
         **/
        void setSendChannel(UnreliableChannel channel){
            this->sendChannel = channel;
        }

        /**
         * setReceiveChannel()
         * 
         * Description:
         *   Called by main to set the unreliable receiving lower-layer channel
         * 
         * 
         **/
        void setReceiveChannel(UnreliableChannel channel){
            this->receiveChannel = channel;
        }

        /**
         * setDataToSend()
         * 
         * Description:
         *   Called by main to set the string data to send
         * 
         * 
         **/
        void setDataToSend(string data){
            this->dataToSend = data;
        }

        /**
         * getDataReceived()
         * 
         * Description:
         *   Called by main to get the currently received and buffered string data, in order
         * 
         * 
         **/
        string getDataReceived(){
            /************************************************************
             * identify the data that has been received...              *
             *                                                          *
             * Add code to this function as needed!                     *
             ************************************************************/
            return assembledData;
        }

        /* ************************************************************************************************************** */
        /* getCountSegmentTimeouts()                                                                                      */
        /*                                                                                                                */
        /* Description:                                                                                                   */
        /* Called by main to get the count of segment timeouts                                                            */
        /*                                                                                                                */
        /*                                                                                                                */
        /* ************************************************************************************************************** */
        int getCountSegmentTimeouts()
        {
            return this->countSegmentTimeouts;
        }

        /**
         * processData()
         * 
         * Description:
         *   "timeslice". Called by main once per iteration
         * 
         * 
         **/
        void processData(){
            this->currentIteration++;
            this->processSend();
            this->processReceiveAndSendResponse();
        }

        /**
         * processSend()
         * 
         * Description:
         *   Manages Segment sending tasks
         * 
         * 
         **/
        void processSend(){
            Segment segmentSend;
            
            /************************************************************
             * Add code to this function as needed!
             * 
             * You should pipeline segments to fit the flow-control window
             * The flow-control window is the constant RDTLayer.FLOW_CONTROL_WIN_SIZE
             * The maximum data that you can send in a segment is RDTLayer.DATA_LENGTH
             * These constants are given in # characters
             * 
             * Somewhere in here you will be creating data segments to send.
             * The data is just part of the entire string that you are trying to send.
             * The seqnum is the sequence number for the segment (in character number, not bytes)
             ************************************************************/

            //send new segments within the window
            while (nextSeqNum < sendBase + FLOW_CONTROL_WIN_SIZE && nextSeqNum < (int)dataToSend.length()) 
            {
                string payload = dataToSend.substr(nextSeqNum, DATA_LENGTH);
                Segment segment;

                segment.setData(nextSeqNum, payload);
                segment.setStartIteration(currentIteration);

                sentSegments[nextSeqNum] = segment;
                sendTimestamps[nextSeqNum] = currentIteration;

                cout << "Sending segment: " << segment.to_string() << endl;
                sendChannel->send(segment);

                nextSeqNum += DATA_LENGTH;
            }

        //retransmit timed out segments
        for (auto& pair : sentSegments) 
        {
            int seq = pair.first;
            Segment& segment = pair.second;

            if (currentIteration - sendTimestamps[seq] >= TIMEOUT) 
            {
                cout << "Timeout! Resending segment: " << segment.to_string() << endl;
                sendChannel->send(segment);
                sendTimestamps[seq] = currentIteration;
                countSegmentTimeouts++;
            }
        }
    }
        

        /**
        * processReceiveAndSendResponse()
        * 
        * Description:
        *   Manages Segment receive tasks
        * 
        * 
        **/
        void processReceiveAndSendResponse(){
            //Segment segmentAck; Segment acknowledging packet(s) received

            //This call returns a list of incoming segments (see Segment class)...
            //vector<Segment> listIncommingSegments = receiveChannel->receive();
            vector<Segment> incoming = receiveChannel->receive();

            int highestAcked = sendBase;
            bool ackSent = false;
            int lastAckSent = -1;

            /*************************************************
             * What segments have been received?
             * How will you get them back in order?
             * This is where a majority of your logic will be implemented
             *************************************************/
            
            /*************************************************
             * How do you respond to what you have received?
             * How can you tell data segments apart from ack segments?
             *************************************************/

            /*************************************************
             * Somewhere in here you will be setting the contents of the ack segments to send.
             * The goal is to employ cumulative ack, just like TCP does...
             *************************************************/

            for (Segment& seg : incoming) 
            {
                if (!seg.checkChecksum()) 
                {
                    cout << "Corrupted segment received. Dropping." << endl;
                    continue;
                }

                //process ACK
                int ackNum = seg.getAck();

                if (ackNum != -1) 
                {
                    if (ackNum > sendBase) 
                    {
                        sendBase = ackNum;

                        //remove acknowledged segments
                        vector<int> toRemove;
                        for (auto& pair : sentSegments) 
                        {
                            if (pair.first < sendBase)
                            {
                                toRemove.push_back(pair.first);
                            }
                        }

                        for (int seq : toRemove) 
                        {
                            sentSegments.erase(seq);
                            sendTimestamps.erase(seq);
                        }
                    }
                    continue;
                }

                //process DATA
                int seq = seg.getAck() == -1 ? seg.to_string().find("seq:") != string::npos ? seg.getStartIteration() : -1 : -1;
                seq = seg.to_string().find("seq: ") != string::npos ? std::stoi(seg.to_string().substr(5, seg.to_string().find(",") - 5)) : seq;
                
                if (seq == -1 || receivedSeqNums.count(seq)) 
                {
                    continue; //duplicate
                }

                receivedSeqNums.insert(seq);
                receiveBuffer[seq] = seg.to_string().substr(seg.to_string().find("data: ") + 6);

                //reconstruct assembled string in order
                while (receiveBuffer.count(expectedSeqNum)) 
                {
                    assembledData += receiveBuffer[expectedSeqNum];
                    receiveBuffer.erase(expectedSeqNum);
                    expectedSeqNum += DATA_LENGTH;
                }

                lastAckSent = expectedSeqNum;
                ackSent = true;
        }

        //send cumulative ACK if necessary
        if (ackSent) 
        {
            Segment ackSegment;
            ackSegment.setAck(lastAckSent);
            cout << "Sending ack: " << ackSegment.to_string() << endl;
            sendChannel->send(ackSegment);
        }
    }

            //int acknum = 0;

            /*
            // Display response segment
            segmentAck.setAck(acknum);
            cout << "Sending ack: " << segmentAck.to_string() << endl;

            // Use the Unreliable sendChannel to send the ack packet
            this->sendChannel->send(segmentAck);
            */
};
