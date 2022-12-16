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
    return result.none();
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
    float totalDelay = PT + TD;
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

int Node::modifyMessage(CustomMessage_Base* msg, std::string command){
    int index = -1;
    if(command[0]=='1'){
        // Not sure yet if we need to use probability to modify the message
        // Uncomment this if we need
        /*
        int prob = uniform(0,1) *10;
        // Modify with prob 80%
        if (prob < 8)
        {
            message[0]=message[0]+1;
        }
        */
       // randomly choose a character to modify depending on the message size
        std::string message = msg->getPayload();
        index = uniform(0,1) * message.size();
        // Convert the char to bitset
        std::bitset<8> b(message[index]);
        // Invert the bit
        b[index%8] = !b[index%8];
        // Convert the bitset back to char
        message[index] = (char)b.to_ulong();
        // Set the modified message
        msg->setPayload(message.c_str());
    }
    // return the modified bit index
    return index;
}

float Node::msgDubDelay(std::string command){//-1 dont send
    float totalDelay = PT + TD + DD;
    if(command[1]=='0' && command[2]=='1' && command[3]=='1'){
        totalDelay+=ED;
    }
    else if(command[1]=='0' && command[2]=='1' && command[3]=='0'){
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
int incrementSeqNum(int seqNum){
    int temp = (seqNum+1)%2;
    return temp;
}
int decrementSeqNum(int seqNum){
    int temp = (seqNum-1)%2;
    if(temp < 0){
        temp = 1;
    }
    return temp;
}
//______________________________________________________________________________________________
void Node::initialize(){
    WS = par("WS");
    TO = par("TO");
    PT = par("PT");
    TD = par("TD");
    ED = par("ED");
    DD = par("DD");
    LP = par("LP");
    EV<<"WS: "<<WS<<", TO: "<<TO<<", PT: "<<PT<<", TD: "<<TD<<", ED: "<<ED<<", DD: "<<DD<<", LP: "<<LP<<endl;
}

void Node::handleMessage(cMessage *msg){
    //Drop the message if the node is still processing the previous message
    if (EndProcessingTimeStamp > simTime()){
        EV <<"A message has been dropped" << endl;
        return;
    }

    EndProcessingTimeStamp = simTime() + processDelay();
    CustomMessage_Base* RecivedMsg = check_and_cast<CustomMessage_Base*>(msg);
    int ReceivedSeqNum = RecivedMsg->getAckNumber();

    // Msg from the coordinator to Sender for initializing the communication (non self message, kind = 0)
    if (!RecivedMsg->isSelfMessage() && RecivedMsg->getKind() == 0){
        EV << "Node "<< getIndex() <<" will act as sender.." << endl;
        EV << "Node "<< 1-getIndex() <<" will act as receiver.." << endl;
        EV << "Node "<< getIndex() <<" will start after: " << RecivedMsg->getPayload() << endl;
        double startTime = double(std::stod(RecivedMsg->getPayload()));
        
        char filePath[2048];
        strcpy(filePath, "../src/input");
        strcat(filePath, std::to_string(getIndex()).c_str());
        strcat(filePath, ".txt");
        infile=fopen(filePath, "r");
        std::string line="";
        char buffer[2];
        while (fgets(buffer, sizeof(buffer), infile) != NULL) {
            if (buffer[0] == '\n') break;
            line += buffer;
        }
        if (line == ""){
            EV << "Sender"<<" finished sending" << endl;
            return;
        }
        std::string command = line.substr(0, line.find(" "));
        std::string payload = line.substr(line.find(" ")+1);

        msgBuffer.payload = payload;
        msgBuffer.command = command;

        scheduleAt(simTime() + startTime + processDelay(), new CustomMessage_Base(""));//The the processing time here is of the next message (not the current one)
    }

    // Sender ready to send a new message (self message, kind = 0)
    else if (RecivedMsg->isSelfMessage() && RecivedMsg->getKind()==0){
        EV << "Sender" << " current seqNum("<<expectedseqNum<<")"<<endl;
        EV << "Sender" << " using command("<< msgBuffer.command <<")" << endl;
        EV << "Sender" << " sending payload("<< msgBuffer.payload <<")" << endl;

        CustomMessage_Base *msgToSend = new CustomMessage_Base("");
        // Calculate the delay
        float delay = msgDelay(msgBuffer.command);
        float dubDelay = msgDubDelay(msgBuffer.command);

        EV << "Sender" << " delay("<< delay <<")" << endl;
        EV << "Sender" << " dubDelay("<< dubDelay <<")" << endl;
        EV << "Sender" << " time out delay("<<timeOutDelay()<<")"<<endl;

        // Apply Framing
        std::string stuffedstr = byteStuffing(msgBuffer.payload);
        // Set the payload with the stuffed message
        msgToSend -> setPayload(stuffedstr.c_str());
        // Add parity
        addParity(msgToSend);
        // Done: Apply error if needed
        // Store the index of changed bit if it is -1 then no error
        int errorIndex = modifyMessage(msgToSend , msgBuffer.command);
        if (errorIndex != -1)
        {
            EV<< "Bit " << errorIndex << " is modified"<<endl;
            EV<< "Sender" << " sending payload after modification("<< msgToSend->getPayload() <<")" << endl;
        }
        // Print the msg to console
        // Set the kind to 1 to differentiate between the coordinator (Kind 0) and the other nodes
        msgToSend->setKind(1);// (non self message, kind = 1)
        msgToSend->setAckNumber(expectedseqNum);
        expectedseqNum = incrementSeqNum(expectedseqNum);
        if (delay != -1){
            // Send the message to the other node if no loss
            sendDelayed(msgToSend, delay, "out");
        }
        if (dubDelay != -1){
            // Send the duplicated message to the other node if exists and no loss
            CustomMessage_Base* msgDub = msgToSend -> dup();
            sendDelayed(msgDub, dubDelay, "out");
        }

        timeoutMsgPtr = new CustomMessage_Base("", 1);
        scheduleAt(simTime() + timeOutDelay(), timeoutMsgPtr);//The timeout for the message (self message, kind = 1)
    }

    // Sender Timer Timeout (self message, kind = 1)
    else if (RecivedMsg->isSelfMessage() && RecivedMsg->getKind()==1){
        EV << "Sender"<<" timeout has happend" << endl;
        expectedseqNum = decrementSeqNum(expectedseqNum);
        //No new line is read, so the same message is sent again
        scheduleAt(EndProcessingTimeStamp, new CustomMessage_Base("", 0));//the processing time for the next message (self message, kind = 0)
    }

    // Sender receives correct ACK (non self message, kind = 2)
    else if (RecivedMsg->getKind() == 2 && ReceivedSeqNum==expectedseqNum){//Sender receives ACK subroutine
        EV << "Sender"<<" received ACK" << endl;

        if (timeoutMsgPtr != nullptr){
            cancelAndDelete(timeoutMsgPtr);
            timeoutMsgPtr = nullptr;
        }

        std::string line="";
        char buffer[2];
        while (fgets(buffer, sizeof(buffer), infile) != NULL) {
            if (buffer[0] == '\n') break;
            line += buffer;
        }
        if (line == ""){
            EV << "Sender"<<" finished sending" << endl;
            return;
        }
        std::string command = line.substr(0, line.find(" "));
        std::string payload = line.substr(line.find(" ")+1);
        msgBuffer.payload = payload;
        msgBuffer.command = command;

        scheduleAt(EndProcessingTimeStamp, new CustomMessage_Base("", 0));//the processing time for the next message (self message, kind = 0)
    }

    // Sender receives NACK (non self message, kind = 3)
    else if (RecivedMsg->getKind() == 3){//Sender receives NACK subroutine
        EV << "Sender"<<" received NACK" << endl;
    }

    // Receiver ready to receive a new message (non self message, kind = 1)
    else if (RecivedMsg->getKind() == 1){//Receiver receives message subroutine
        // Remove escape characters from the payload
        std::string receivedMsg = RecivedMsg->getPayload();
        // Check the parity
        bool isNotCorrupt = checkParity(RecivedMsg);
        
        // If the message is not corrupt remove the escape characters
        if(isNotCorrupt){
            EV << "Receiver"<<" received message is not corrupt" << endl;
            EV << "Receiver" << " current seqNum("<<ReceivedSeqNum<<")"<<endl;
            EV << "Receiver" << " expected seqNum("<<expectedseqNum<<")"<<endl;
            EV << "Receiver"<<" received payload("<< receivedMsg <<")" << endl;
            EV << "Receiver" << " delay("<<PT+TD<<")"<<endl;
            if(ReceivedSeqNum == expectedseqNum){//If the message seq num is the expected one increment the expected seqNum
                expectedseqNum = incrementSeqNum(expectedseqNum);//because the sender wants the next message with the next seqNum
            }
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
            // Send ACK (non self message, kind = 2)
            if (!isLost()){//Send ACK
                CustomMessage_Base* ackmsg = new CustomMessage_Base("");
                ackmsg->setAckNumber(expectedseqNum);
                ackmsg->setKind(2);
                sendDelayed(ackmsg, PT+TD, "out");
            }
        }
        // Send NACK (non self message, kind = 3)
        else{
            EV << "Receiver"<<" received message is corrupt" << endl;
            CustomMessage_Base* nackmsg = new CustomMessage_Base("");
            nackmsg->setAckNumber(expectedseqNum);
            nackmsg->setKind(3);
            sendDelayed(nackmsg, PT+TD, "out");
        }
    }
}
