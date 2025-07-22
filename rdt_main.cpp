#pragma once

#include <string>
#include <sstream>
#include <iostream>

#include "rdt_layer.cpp"
#include "unreliable.cpp"

using std::string;
using std::ostringstream;
using std::cout;
using std::cin;

/************************************************************
 * Main
 * 
 * 
 ************************************************************/
int main(int argc, char **argv){
    /************************************************************
     * The following are two sets of data input to the communication test. The first is small and the second is longer.
     * Start by uncomming the shorter until you feel you have a good algorithm. Then confirm it still works on a larger
     * scale by switching to the larger.
     * 
     ************************************************************/

    string dataToSend = "";
    
    //use this for short input
    string shortSentence = "The quick brown fox jumped over the lazy dog";

    //use this for long input
    ostringstream oss;
    oss << "\r\n\r\n...We choose to go to the moon. We choose to go to the moon in this "
        "decade and do the other things, not because they are easy, but because they are hard, "
        "because that goal will serve to organize and measure the best of our energies and skills, "
        "because that challenge is one that we are willing to accept, one we are unwilling to "
        "postpone, and one which we intend to win, and the others, too."
        "\r\n\r\n"
        "...we shall send to the moon, 240,000 miles away from the control station in Houston, a giant "
        "rocket more than 300 feet tall, the length of this football field, made of new metal alloys, "
        "some of which have not yet been invented, capable of standing heat and stresses several times "
        "more than have ever been experienced, fitted together with a precision better than the finest "
        "watch, carrying all the equipment needed for propulsion, guidance, control, communications, food "
        "and survival, on an untried mission, to an unknown celestial body, and then return it safely to "
        "earth, re-entering the atmosphere at speeds of over 25,000 miles per hour, causing heat about half "
        "that of the temperature of the sun--almost as hot as it is here today--and do all this, and do it "
        "right, and do it first before this decade is out.\r\n\r\n"
        "JFK - September 12, 1962\r\n";
    string longParagraph = oss.str();

    dataToSend = shortSentence;
    //dataToSend = longParagraph;

    RDTLayer client;
    RDTLayer server;

    //Start with a reliable channel (all flags false)
    //As you create your rdt algorithm for send and receive, turn these on.

    bool outOfOrder = false;
    bool dropPackets = false;
    bool delayPackets = false;
    bool dataErrors = false;

    //Create unreliable communication channels
    UnreliableChannel clientToServerChannel(outOfOrder,dropPackets,delayPackets,dataErrors);
    UnreliableChannel serverToClientChannel(outOfOrder,dropPackets,delayPackets,dataErrors);

    //Creat client and server that connect to unreliable channels
    client.setSendChannel(clientToServerChannel);
    client.setReceiveChannel(serverToClientChannel);
    server.setSendChannel(serverToClientChannel);
    server.setReceiveChannel(clientToServerChannel);

    //Set initial data that will be sent from client to server
    client.setDataToSend(dataToSend);

    int loopIter = 0;            //Used to track communication timing in iterations
    while(true){
        cout << "-----------------------------------------------------------------------------------------------------------" << endl;
        loopIter++;
        cout << "Time (iterations) = " << loopIter << endl;

        //Sequence to pass segments back and forth between client and server
        cout << "Client------------------------------------------" << endl;
        client.processData();
        clientToServerChannel.processData();
        cout << "Server------------------------------------------" << endl;
        server.processData();
        serverToClientChannel.processData();

        //show the data received so far
        cout << "Main--------------------------------------------" << endl;
        string dataReceivedFromClient = server.getDataReceived();
        cout << "DataReceivedFromClient: " << dataReceivedFromClient << endl;

        if(dataReceivedFromClient == dataToSend){
            cout << "$$$$$$$$ ALL DATA RECEIVED $$$$$$$$" << endl;
            break;
        }

        //Used max streamsize looking for the newline character since we specifically say we are looking for the entr key...
        cout << "Press enter to continue...";
        cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
    }

    cout << "countTotalDataPackets: " << clientToServerChannel.getCountTotalDataPackets() << endl;
    cout << "countSentPackets: " << clientToServerChannel.getCountSentPackets() + serverToClientChannel.getCountSentPackets() << endl;
    cout << "countChecksumErrorPackets: " << clientToServerChannel.getCountChecksumErrorPackets() << endl;
    cout << "countOutOfOrderPackets: " << clientToServerChannel.getCountOutOfOrderPackets() << endl;
    cout << "countDelayedPackets: " << clientToServerChannel.getCountDelayedPackets() + serverToClientChannel.getCountDelayedPackets() << endl;
    cout << "countDroppedDataPackets: " << clientToServerChannel.getCountDroppedPackets() << endl;
    cout << "countAckPackets: " << serverToClientChannel.getCountAckPackets() << endl;
    cout << "countDroppedAckPackets: " << serverToClientChannel.getCountDroppedPackets() << endl;

    cout << "# segment timeouts: " << client.getCountSegmentTimeouts() << endl;

    cout << "TOTAL ITERATIONS: " << loopIter << endl;

    return 0;
}
