from segment import Segment


# #################################################################################################################### #
# RDTLayer                                                                                                             #
#                                                                                                                      #
# Description:                                                                                                         #
# The reliable data transfer (RDT) layer is used as a communication layer to resolve issues over an unreliable         #
# channel.                                                                                                             #
#                                                                                                                      #
#                                                                                                                      #
# Notes:                                                                                                               #
# This file is meant to be changed.                                                                                    #
#                                                                                                                      #
#                                                                                                                      #
# #################################################################################################################### #


class RDTLayer(object):
    # ################################################################################################################ #
    # Class Scope Variables                                                                                            #
    #                                                                                                                  #
    #                                                                                                                  #
    #                                                                                                                  #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    DATA_LENGTH = 4 # in characters                     # The length of the string data that will be sent per packet...
    FLOW_CONTROL_WIN_SIZE = 15 # in characters          # Receive window size for flow-control
    sendChannel = None
    receiveChannel = None
    dataToSend = ''
    currentIteration = 0                                # Use this for segment 'timeouts'
    # Add items as needed
    window =[0,4]
    sequenceNum = 0
    data = []
    ack = 4
    

    # ################################################################################################################ #
    # __init__()                                                                                                       #
    #                                                                                                                  #
    #                                                                                                                  #
    #                                                                                                                  #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    def __init__(self):
        self.sendChannel = None
        self.receiveChannel = None
        self.dataToSend = ''
        self.currentIteration = 0
        # Add items as needed
        self.countSegmentTimeouts = 0
        self.mode = "Server"
        self.minWin = 0
        self.maxWin = 4
        self.segmentTimer = 0
        self.currAck = 0
        
    # ################################################################################################################ #
    # setSendChannel()                                                                                                 #
    #                                                                                                                  #
    # Description:                                                                                                     #
    # Called by main to set the unreliable sending lower-layer channel                                                 #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    def setSendChannel(self, channel):
        self.sendChannel = channel

    # ################################################################################################################ #
    # setReceiveChannel()                                                                                              #
    #                                                                                                                  #
    # Description:                                                                                                     #
    # Called by main to set the unreliable receiving lower-layer channel                                               #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    def setReceiveChannel(self, channel):
        self.receiveChannel = channel

    # ################################################################################################################ #
    # setDataToSend()                                                                                                  #
    #                                                                                                                  #
    # Description:                                                                                                     #
    # Called by main to set the string data to send                                                                    #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    def setDataToSend(self,data):
        self.dataToSend = data

    # ################################################################################################################ #
    # getDataReceived()                                                                                                #
    #                                                                                                                  #
    # Description:                                                                                                     #
    # Called by main to get the currently received and buffered string data, in order                                  #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    def getDataReceived(self):
        # ############################################################################################################ #
        # Identify the data that has been received...
        
        sortedData = sorted(self.data)

        sortedString = ""
        for i in range(len(sortedData)):
            sortedString += sortedData[i][1]
        
        return sortedString

    # ################################################################################################################ #
    # processData()                                                                                                    #
    #                                                                                                                  #
    # Description:                                                                                                     #
    # "timeslice". Called by main once per iteration                                                                   #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    def processData(self):
        self.currentIteration += 1
        self.processSend()
        self.processReceiveAndSendRespond()

    # ################################################################################################################ #
    # processSend()                                                                                                    #
    #                                                                                                                  #
    # Description:                                                                                                     #
    # Manages Segment sending tasks                                                                                    #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    def processSend(self):

        #print current size of the receive queue
        print(f"Client Length of Receive Unacked Packets List: {len(self.receiveChannel.receiveQueue)}")
        if(self.dataToSend != ""):
            self.mode = "Client"

        split_data = [self.dataToSend[i:i + self.DATA_LENGTH] for i in range(0, len(self.dataToSend), self.DATA_LENGTH)]

        #handle if no ACKs received after several iterations
        if(self.currentIteration > 1 and len(self.receiveChannel.receiveQueue ) == 0):
            
            if(self.segmentTimer == 3):
                self.sequenceNum = self.window[0]
                self.countSegmentTimeouts += 1
            else:
                self.segmentTimer += 1
                return

        #if ACKs have been received in client mode then check them
        if (len(self.receiveChannel.receiveQueue) > 0 and self.mode == "Client"):
            acklist = self.receiveChannel.receive()
            self.ackCheck(acklist)

        seqnum = self.sequenceNum
        self.minWin = seqnum
        self.maxWin = seqnum + 4

        #only send if we are the client
        if(self.mode != "server"):
            self.sendUnackedSegments(self.minWin, self.maxWin, seqnum, split_data)

    def ackCheck(self, toCheck):

        #loop through received ACKs and check if expected ACK number is present
        for i in range(0, len(toCheck)):
            if(toCheck[i].acknum == self.ack):
                self.sequenceNum += 4
                self.ack += 4
                self.window[0] += 4
                self.window[1] += 4
        return

    def sendUnackedSegments(self, start, end, seqnum, dataArray):

        #send segments in the window range    
        for i in range(start, end):
            if (self.dataToSend != "" and seqnum < len(dataArray)):
                segmentSend = Segment()
                data = dataArray[seqnum]

                #prepare and fill the segment with data
                segmentSend.setData(seqnum, data)
                seqnum += 1
                segmentSend.setStartIteration(self.currentIteration)
                segmentSend.setStartDelayIteration(4)
                
                print(f"Sending segment: seq: {segmentSend.seqnum}, ack: -1, data: {segmentSend.payload}")

                self.sendChannel.send(segmentSend)
        return

    # ################################################################################################################ #
    # processReceive()                                                                                                 #
    #                                                                                                                  #
    # Description:                                                                                                     #
    # Manages Segment receive tasks                                                                                    #
    #                                                                                                                  #
    #                                                                                                                  #
    # ################################################################################################################ #
    def processReceiveAndSendRespond(self):
        segmentAck = Segment()

        # This call returns a list of incoming segments (see Segment class)...
        listIncomingSegments = self.receiveChannel.receive()

        # ############################################################################################################ #
        # What segments have been received?
        # How will you get them back in order?
        # This is where a majority of your logic will be implemented
        
        print(f"Server Length of Receive Unacked Packets List: {len(listIncomingSegments)}")

        #check if there are any incoming segments to process
        if(len(listIncomingSegments) > 0):
            currentAck = self.window[0]
            self.ack = self.window[1]
            newList, recAck = self.sortList(listIncomingSegments)
            currentAck += recAck

            #checlk if all expected packets in the window have been received
            if(currentAck == self.ack):
                self.minWin += 4
                self.currAck = self.currAck + 4
                segmentAck.setAck(currentAck)
                self.sendChannel.send(segmentAck)

            self.appendData(newList)
        else:
            return

    def sortList(self, sort):

        sequenceList = []
        processOriginal = []

        #filter the incoming segments for non-empty payloads and valid checksums
        for i in range(len(sort)):
            if(sort[i].payload!="" and sort[i].checkChecksum() == True):
                sequenceList.append([sort[i].seqnum, sort[i].payload])

        #keep only new and in order packets within the current receive window
        for j in range(len(sequenceList)):
            if(sequenceList[j] not in processOriginal and (self.window[0]<=sequenceList[j][0] <= self.window[1])):
                processOriginal.append(sequenceList[j])

        return processOriginal, len(processOriginal)

    def appendData(self, append):

        #add each unique sequence number and payload entry to the data
        for i in range(len(append)):
            if(append[i] not in self.data):
                self.data.append(append[i])