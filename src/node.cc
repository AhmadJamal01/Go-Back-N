//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "node.h"
#include "CustomMessage_m.h"

Define_Module(Node);

void Node::addParity (CustomMessage_Base* msg){
    std::string payload = msg->getPayload();
    std::bitset<8> result(0);
    // XOR each character with the previous one
    for (int i = 0; i<payload.size();i++){
        std::bitset<8> b(payload[i]);
        result = result ^ b;
    }
    char resChar = (char)result.to_ulong();
    msg ->setTrailer(resChar);
}

bool Node::checkParity (CustomMessage_Base* msg){
    std::string payload = msg->getPayload();
    std::bitset<8> result(0);
    for (int i = 0; i<payload.size();i++){
        std::bitset<8> b(payload[i]);
        result = result ^ b;
    }
    std::bitset<8> parity(msg -> getTrailer());
    result = result ^ parity;
    return !result.all();
}

std::string Node::byteStuffing(std::string message)
{
     // Handle flag and escape characters in the payload
     // And add the flag to the start and end of the message
    std::vector<char> stuffedmsg;
    stuffedmsg.push_back(FLAG);
    for (int i = 0; i < message.size(); i++)
     {
         if (message[i] != FLAG && message[i] != ESCAPE)
         {
             stuffedmsg.push_back(message[i]);
         }
         else
         {
             stuffedmsg.push_back(ESCAPE);
             stuffedmsg.push_back(message[i]);
         }
     }
     stuffedmsg.push_back(FLAG);
     // Convert the vector to string
     std::string stuffedstr(stuffedmsg.begin(), stuffedmsg.end());
     return stuffedstr;
}

float Node::msgDelay(std::string command){//-1 dont send
    float totalDelay = TD;
    if(command[1]=='0' && command[3]=='1'){
        totalDelay+=ED;
    }
    else if(command[1]=='0' && command[3]=='0'){
        totalDelay+=0;
    }
    else{
        totalDelay=-1;
    }
    return totalDelay;
}

float Node::msgDubDelay(std::string command){//-1 dont send
    float totalDelay = TD + DD;
    if(command[1]=='0' && command[3]=='1' && command[2]=='1'){
        totalDelay+=ED;
    }
    else if(command[1]=='0' && command[3]=='0' && command[2]=='1'){
        totalDelay+=0;
    }
    else{
        totalDelay=-1;
    }
    return totalDelay;
}

float Node::processDelay(){
    return PT;
}

float Node::timeOutDelay(){
    return TO;
}

bool Node::isLost(){
    //randomly return true with probability LP (with precision of 2 decimal places)
    return (rand()%100 < int(LP*100));
}

//______________________________________________________________________________________________
void Node::initialize(){
}

void Node::handleMessage(cMessage *msg){
    //Drop the message if the node is still processing the previous message
    if (EndProcessingTimeStamp > simTime()){
        return;
    }

    EndProcessingTimeStamp = simTime() + processDelay();
    CustomMessage_Base* RecivedMsg = check_and_cast<CustomMessage_Base*>(msg);
    int ReceivedSeqNum = RecivedMsg->getAckNumber();

    // Msg from the coordinator to Sender initializing the communication (non self message, kind = 0)
    if (!RecivedMsg->isSelfMessage() && RecivedMsg->getKind() == 0){
        EV << "Node "<< getIndex() <<" will act as sender.." << endl;
        EV << "Node "<< 1-getIndex() <<" will act as receiver.." << endl;
        EV << "Node "<< getIndex() <<" will start after: " << RecivedMsg->getPayload() << endl;
        double startTime = double(std::stod(RecivedMsg->getPayload()));

        std::string command = "0000";//TODO: MESSAGE = getCOMMANDFromLineString(line);
        std::string payload = "Kak$ka/k";//TODO: MESSAGE = getMsgFromLineString(line);
        msgBuffer.payload = payload;
        msgBuffer.command = command;

        scheduleAt(simTime() + startTime + processDelay(), new CustomMessage_Base(""));//The the processing time here is of the next message (not the current one)
    }

    // Sender ready to send a new message (self message, kind = 0)
    else if (RecivedMsg->isSelfMessage() && RecivedMsg->getKind()==0){
        CustomMessage_Base *msgToSend = new CustomMessage_Base("");
        EV << "Node " << getIndex() <<" Started ..." << endl;
        // Calculate the delay
        float delay = msgDelay(msgBuffer.command);
        float dubDelay = msgDubDelay(msgBuffer.command);
        // Apply Framing
        std::string stuffedstr = byteStuffing(msgBuffer.payload);
        // Set the payload with the stuffed message
        msgToSend -> setPayload(stuffedstr.c_str());
        // Add parity
        addParity(msgToSend);
        // TODO: Apply error if needed
        // Print the msg to console
        EV << "Node " << getIndex() << " sent " << msgToSend -> getPayload() << endl;
        EV << msgToSend -> getTrailer() << endl;
        // Set the kind to 1 to differentiate between the coordinator (Kind 0) and the other nodes
        msgToSend->setKind(1);// (non self message, kind = 1)
        msgToSend->setAckNumber(expectedseqNum);
        expectedseqNum = (expectedseqNum+1)%2;
        if (delay != -1){
            // Send the message to the other node if no loss
            sendDelayed(msgToSend, delay, "out");
        }
        if (dubDelay != -1){
            // Send the duplicated message to the other node if exists and no loss
            CustomMessage_Base* msgDub = msgToSend -> dup();
            sendDelayed(msgDub, dubDelay, "out");
        }
        scheduleAt(simTime() + timeOutDelay(), new CustomMessage_Base("", 1));//The timeout for the message (self message, kind = 1)
    }

    // Sender Timer Timeout (self message, kind = 1)
    else if (RecivedMsg->isSelfMessage() && RecivedMsg->getKind()==1){
        //No new line is read, so the same message is sent again
        scheduleAt(EndProcessingTimeStamp, new CustomMessage_Base("", 0));//the processing time for the next message (self message, kind = 0)
    }

    // Sender receives correct ACK (non self message, kind = 2)
    else if (RecivedMsg->getKind() == 2 && ReceivedSeqNum==expectedseqNum){//Sender receives ACK subroutine
        // Read the next line from the file
        std::string command = "0000";//TODO: MESSAGE = getCOMMANDFromLineString(line);
        std::string payload = "saad";//TODO: MESSAGE = getMsgFromLineString(line);
        msgBuffer.payload = payload;
        msgBuffer.command = command;
        EV<<"AT time "<<simTime()<<"Node"<<getIndex()<<", Introducing channel error with code"<<msgBuffer.command<<endl;
        scheduleAt(EndProcessingTimeStamp, new CustomMessage_Base("", 0));//the processing time for the next message (self message, kind = 0)
    }

    // Sender receives NACK (non self message, kind = 3)
    else if (RecivedMsg->getKind() == 3){//Sender receives NACK subroutine
    }

    // Receiver ready to receive a new message (non self message, kind = 1)
    else if (RecivedMsg->getKind() == 1){
        // Remove escape characters from the payload
        std::string receivedMsg = RecivedMsg->getPayload();
        // Check the parity
        bool isNotCorrupt = checkParity(RecivedMsg);
        EV << "Parity Check: "<< isNotCorrupt << endl;
        // If the message is not corrupt remove the escape characters
        if(isNotCorrupt){
            std::vector<char> msgWithoutEsc;
            bool isPrevEsc = false;
            for (int i=1 ; i< receivedMsg.size()-1 ; i++){
                if (isPrevEsc){
                    msgWithoutEsc.push_back(receivedMsg[i]);
                    isPrevEsc = false ;
                    continue;
                }
                else if (!isPrevEsc && receivedMsg[i]==ESCAPE){
                    isPrevEsc = true;
                    continue;
                }
                msgWithoutEsc.push_back(receivedMsg[i]);
            }
            // Convert vector back to string
            std::string msgWithoutEscStr(msgWithoutEsc.begin(), msgWithoutEsc.end());
            EV << "Received after removing escape characters: "<< msgWithoutEscStr << endl;
            //Send ACK with kind 2
            if (!isLost()){//Send ACK
                CustomMessage_Base* ackmsg = new CustomMessage_Base("");
                expectedseqNum = (expectedseqNum+1)%2; //because the sender wants the next message with the next seqNum
                ackmsg->setAckNumber(expectedseqNum);
                ackmsg->setKind(2);
                sendDelayed(ackmsg, PT+TD, "out");
            }
        }
        else{//Send ACK with kind 3
            CustomMessage_Base* nackmsg = new CustomMessage_Base("");
            expectedseqNum = (expectedseqNum+1)%2; //because the sender wants the next message with the next seqNum
            nackmsg->setAckNumber(expectedseqNum);
            nackmsg->setKind(3);
            sendDelayed(nackmsg, PT+TD, "out");
        }
    }
}
