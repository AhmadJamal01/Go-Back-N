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
int Node::dist(int x, int y){
    int count=0;
    while(x!=y){
        x=(x+1)%(WS+1);
        count++;
    }
    return count;
}
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

float Node::msgDelay(std::string command){//-1 == dont send (lost)
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

float Node::msgDubDelay(std::string command){//-1 dont send (lost)
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
    // Randomly return true with probability LP (with precision of 2 decimal places)
    return (rand()%100 < int(LP*100));
}
int Node::incrementSeqNum(int seqNum){
    int temp = (seqNum+1)%(WS+1);
    return temp;
}

std::string Node::removeEsc(std::string receivedMsg){
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
    std::string msgWithoutEscStr(msgWithoutEsc.begin(), msgWithoutEsc.end());// Convert vector back to string
    return msgWithoutEscStr;
    
}

void Node::print1(omnetpp::SimTime processingTime, int index, std::string command){
//    auto ss = std::stringstream();
//    ss << "==At time ["<< processingTime << "], Node [" << index << "], Introducing channel error with code = ["<< command <<"]\n";
//    auto msg = ss.str();
//
//    outfile << msg;
//    outfile.flush();
//
//    sender_buffer += msg;
//    EV << msg;
    outfile << "At time ["<< processingTime << "], Node [" << index << "], Introducing channel error with code = ["<< command <<"]\n";
    EV << "At time ["<< processingTime << "], Node [" << index << "], Introducing channel error with code = ["<< command <<"]\n";
}

void Node::print2(omnetpp::SimTime processingTime, int index, std::string action, int seqNum, std::string payload, char trailer, int modified, std::string lost, int duplicate, int delay){
    outfile << "At time ["<< processingTime << "], Node [" << index << "], [" << action << "] frame with seq_num = [" << seqNum << "] and payload = [" << payload << "] and trailer = [" << trailer << "], Modified [" << modified << "] Lost [ " << lost << "], Duplicate [" << duplicate << "], Delay [" << delay << "]" << endl;
    EV << "At time ["<< processingTime << "], Node [" << index << "], [" << action << "] frame with seq_num = [" << seqNum << "] and payload = [" << payload << "] and trailer = [" << trailer << "], Modified [" << modified << "] Lost [ " << lost << "], Duplicate [" << duplicate << "], Delay [" << delay << "]" << endl;
}

void Node::print3(omnetpp:: SimTime timeoutTime, int index, int seqNum){
    outfile << "Time out event at time [" << timeoutTime << "], at Node [ " << index << "] for frame with seq_num = [" << seqNum << "]" << endl;
    EV << "Time out event at time [" << timeoutTime << "], at Node [ " << index << "] for frame with seq_num = [" << seqNum << "]" << endl;
}

void Node::print4(omnetpp::SimTime processingTime, int index, std::string n_ack, int num, std::string loss){
    outfile << "At time [" << processingTime << "], Node [" << index << "], Sending [" << n_ack << "] with number [" << num << "], loss [" << loss << "]" << endl;
    EV << "At time [" << processingTime << "], Node [" << index << "], Sending [" << n_ack << "] with number [" << num << "], loss [" << loss << "]" << endl;
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
    //EV<<"WS: "<<WS<<", TO: "<<TO<<", PT: "<<PT<<", TD: "<<TD<<", ED: "<<ED<<", DD: "<<DD<<", LP: "<<LP<<endl;

    // initialize the output file
    // may be moved to handleMessage
//    outfile.open("../src/output.txt", std::ofstream::out | std::ofstream::trunc);
    outfile.open("../src/output.txt", std::ios_base::app);
//    outfile.clear();
//    outfile.close();
//    outfile.open("../src/output.txt", std::ios_base::app);
}

void Node::handleMessage(cMessage *msg){
    // Drop the message if the node is still processing the previous message
    if (EndProcessingTime > simTime()){
        //EV <<"A message has been dropped" << endl;
        cancelAndDelete(msg);
        return;
    }
    // Get the data from the received frame (msg, seqNum)
    CustomMessage_Base* RecivedMsg = check_and_cast<CustomMessage_Base*>(msg);
    int ReceivedSeqNum = RecivedMsg->getAckNumber();

    // Msg from the coordinator to Sender for initializing the communication (non self message, kind = 0)
    if (!RecivedMsg->isSelfMessage() && RecivedMsg->getKind() == 0){
        //EV << "Node "<< getIndex() <<" will act as sender.." << endl;
        //EV << "Node "<< 1-getIndex() <<" will act as receiver.." << endl;
        //EV << "Node "<< getIndex() <<" will start after: " << RecivedMsg->getPayload() << endl;
        double startTime = double(std::stod(RecivedMsg->getPayload()));
        // Inti the file for reading the input
        char filePath[2048];
        strcpy(filePath, "../src/input");
        // strcat(filePath, std::to_string(getIndex()).c_str());
        strcat(filePath, ".txt");
        infile=fopen(filePath, "r");
        // Read the first WS messages from the file
        // Get the a line from the file
        for (int i=0; i<WS; i++){
            std::string line="";
            char buffer[2];
            while (fgets(buffer, sizeof(buffer), infile) != NULL) {
                if (buffer[0] == '\n') break;
                line += buffer;
            }
            // If the line is empty, break (no more messages to send)
            if (line == ""){
                //EV << "Sender"<<" No new Msg" << endl;
                break;
            }
            std::string command = line.substr(0, line.find(" "));
            std::string payload = line.substr(line.find(" ") + 1);
            msgBuffer[i].command = command;
            msgBuffer[i].payload = payload;
            S_end= incrementSeqNum(S_end);

            // according to TA's output file, messages in a window are read and sent at the same time
            // scheduleAt(simTime() + startTime + i * processDelay(), new CustomMessage_Base("", 0));// Schedule a message is (self message, kind = 0)
            // outfile << "At ["<< simTime() + startTime + i * processDelay() << "], Node [" << getIndex() << "], Introducing channel error with code = ["<< command <<"]" << endl;
            // EV << "At ["<< simTime() + startTime + i * processDelay() << "], Node [" << getIndex() << "], Introducing channel error with code = ["<< command <<"]" << endl;
            print1(simTime() + startTime + i*processDelay(), getIndex(), command);
            scheduleAt(simTime() + startTime + i*processDelay(), new CustomMessage_Base("", 0));// Schedule a message is (self message, kind = 0)
            // Send the first WS messages (a window)
        }
    }

    // Sender ready to send a new message (self message, kind = 0)
    else if (RecivedMsg->isSelfMessage() && RecivedMsg->getKind()==0){
        // params for the output file
        // may not use some of them
        int printTime = -1;
        std::string nodeStatus = "sent";
        int modified = -1;
        std::string lost = "No";
        int duplicate = -1;
        int errorDelayInterval = -1;
        int duplicateVersion = 0;

        // Specify the time at wich the sender will be free
        EndProcessingTime = simTime() + processDelay();
        //EV << "Sender" << " current seqNum("<<S<<")"<<endl;
        //EV << "Sender" << " using command("<< msgBuffer[S].command <<")" << endl;
        //EV << "Sender" << " sending payload("<< msgBuffer[S].payload <<")" << endl;
        // Prepare the message to send (delay calculation, add framing, add parity, add error)
        CustomMessage_Base *msgToSend = new CustomMessage_Base("");
        float delay = msgDelay(msgBuffer[S].command);// Calculate the delay of the message
        float dubDelay = msgDubDelay(msgBuffer[S].command);// Calculate the delay of the dublicate message
        
        if(dubDelay != -1){
            duplicateVersion = 1;
        }

        //EV << "Sender" << " delay("<< delay <<")" << endl;
        //EV << "Sender" << " dubDelay("<< dubDelay <<")" << endl;
        //EV << "Sender" << " time out delay("<<timeOutDelay()<<")"<<endl;
        // Apply Framing
        std::string stuffedstr = byteStuffing(msgBuffer[S].payload);
        msgToSend -> setPayload(stuffedstr.c_str());// Set the payload with the stuffed message
        addParity(msgToSend);// Add parity
        // Done: Apply error if needed
        // Store the index of changed bit if it is -1 then no error
        int errorIndex = modifyMessage(msgToSend , msgBuffer[S].command);
        if (errorIndex != -1)
        {
            //EV<< "Bit " << errorIndex << " is modified"<<endl;
            //EV<< "Sender" << " sending payload after modification("<< msgToSend->getPayload() <<")" << endl;
            modified = errorIndex;
        }

        // print here with duplicate versions, handle the version numbers yourself spaghetti
        if(msgBuffer[S].command[1]=='0' && msgBuffer[S].command[3]=='1'){
            errorDelayInterval = ED;
        } else {
            errorDelayInterval = 0;
        }
        if(dubDelay != -1){
            // duplicate version 1
            print2(EndProcessingTime, getIndex(), "sent" , S, msgToSend->getPayload(), msgToSend->getTrailer(), errorIndex, "No", 1, errorDelayInterval);
        } else {
            // duplicate version 0
            print2(EndProcessingTime, getIndex(), "sent" , S, msgToSend->getPayload(), msgToSend->getTrailer(), errorIndex, "No", 0, errorDelayInterval);
        }

        msgToSend->setKind(1);// Send a message (non self message, kind = 1)
        msgToSend->setAckNumber(S);
        S = incrementSeqNum(S);
        if (delay != -1){
            // Send the message to the other node if no loss
            sendDelayed(msgToSend, delay, "out");
            //outfile << "At tune [" << simTime() + delay << "] Node [" << getIndex() << "] [sent] frame with seq_num = [" << msgToSend->getAckNumber() << "] and payload = [" << msgToSend->getPayload() << "] and trailer = [" << msgToSend->getTrailer() << "], Modified [" << errorIndex << "], Lost [No], Duplicate;
//            printTime = simTime() + delay;
            // print here with duplicate version 1 x
        }
        else{
            cancelAndDelete(msgToSend);// Delete the message if lost
            lost = "Yes";
        }
        if (dubDelay != -1){
            // Send the duplicated message to the other node if exists and no loss
            duplicateVersion = 2;
            // print here with duplicate version 2 x
            CustomMessage_Base* msgDub = msgToSend -> dup();
            sendDelayed(msgDub, dubDelay, "out");
            print2(EndProcessingTime + DD, getIndex(), "sent" , S, msgDub->getPayload(), msgDub->getTrailer(), errorIndex, "No", 2, errorDelayInterval);

        }
        // Reschedule the timer (every new send)
        if (timeoutMsgPtr!=nullptr){
            cancelAndDelete(timeoutMsgPtr);
            timeoutMsgPtr=nullptr;
        }
        timeoutMsgPtr = new CustomMessage_Base("", 1);
        scheduleAt(simTime() + timeOutDelay(), timeoutMsgPtr);//The timeout for the message (self message, kind = 1)
        ackExpected = S;
    }

    // Sender Timer Timeout (self message, kind = 1)
    else if (RecivedMsg->isSelfMessage() && RecivedMsg->getKind()==1){
        
        // print here time out 
        print3(simTime(), getIndex(),S);

        //EV << "Sender"<<" timeout has happend" << endl;
        S = S_start;
        // Send the whole window
        //EV << "Sender"<<" S_start: " << S_start << " S_end: " << S_end << endl;
        int acksNum = dist(S_start, S_end);
        // Set the fisrt frame command to zeros
        msgBuffer[S_start].command = "0000";
        //for(int i = 0; i < WS; i++){
        //    EV <<" command: " << msgBuffer[(S+i)%(WS+1)].command << " payload: " << msgBuffer[(S+i)%(WS+1)].payload << endl;
        //}
        for(int i = 0; i < acksNum; i++){
            scheduleAt(simTime() + i * processDelay(), new CustomMessage_Base("", 0));// Schedule a message is (self message, kind = 0)
        }
    }

    // Sender receives correct ACK (non self message, kind = 2) [Note: could be acumulative]
    else if (RecivedMsg->getKind() == 2){
        //EV << "Sender"<<" received ACK " <<ReceivedSeqNum << endl;
        //EV << "Sender"<<" S_start "<<S_start << endl;
        //EV << "Sender"<<" S_end "<<S_end << endl;
        //Advance the window based on the received Ack [Note: loop is in case of acumulative ACK else it will run only once]
        int acksNum = dist(S_start, ReceivedSeqNum);
        //EV << "Sender"<<" acksNum "<<acksNum << endl;
        for(int i = 0; i < acksNum; i++){
            S_start = incrementSeqNum(S_start);
            std::string line="";
            // Get the next line from the file
            char buffer[2];
            while (fgets(buffer, sizeof(buffer), infile) != NULL) {
                if (buffer[0] == '\n') break;
                line += buffer;
            }
            // If the line is not empty load it to the buffer
            if (line != ""){
                std::string command = line.substr(0, line.find(" "));
                std::string payload = line.substr(line.find(" ")+1);
                msgBuffer[S_end].payload = payload;
                msgBuffer[S_end].command = command;
                S_end=incrementSeqNum(S_end);

                // print here reading from file
                print1(simTime() + i * processDelay(), getIndex(), command);
            }else{
                // end of file? tick
                // close output file
                // EV << "==============================================" << endl;
                // EV << sender_buffer << endl;
                outfile.close();
            }
            if(S_start != S_end){
                // print here sending x
                // print2(outfile, simTime() + i * processDelay(), getIndex(), "sent" , S, msgBuffer[S].payload, msgBuffer[S].command, errorIndex, "No", 0, 0);
                scheduleAt(simTime() + i * processDelay(), new CustomMessage_Base("", 0));
            }
        }
        // Cancel the timer if the whole window has been acked [Note: that S_start has been updated upove]
        if (S_start == ackExpected){
            if (timeoutMsgPtr != nullptr){
                cancelAndDelete(timeoutMsgPtr);
                timeoutMsgPtr = nullptr;
            }
        }
    }

    // Sender receives NACK (non self message, kind = 3)
    else if (RecivedMsg->getKind() == 3){
        //Do nothing (the timer will resend the whole window)
        //EV << "Sender"<<" received NACK" << endl;
    }

    // Receiver ready to receive a new message (non self message, kind = 1)
    else if (RecivedMsg->getKind() == 1){
        EndProcessingTime = simTime() + processDelay();// Set the end processing time (nothing is received until then)
        std::string receivedMsg = RecivedMsg->getPayload();// Remove escape characters from the payload
        bool isNotCorrupt = checkParity(RecivedMsg);// Check the parity
        // If the message is not corrupt (send the appropriate ACK)
        if(isNotCorrupt){
            //EV << "Receiver"<<" received message is not corrupt" << endl;
            //EV << "Receiver" << " current seqNum("<<ReceivedSeqNum<<")"<<endl;
            //EV << "Receiver" << " expected seqNum("<<R<<")"<<endl;
            //EV << "Receiver"<<" received payload("<< receivedMsg <<")" << endl;
            //EV << "Receiver" << " delay("<<PT+TD<<")"<<endl;
            if(ReceivedSeqNum == R){//If the message seq num is the expected one increment the expected seqNum
                // print here sending ackx
                
                R = incrementSeqNum(R);//because the sender wants the next message with the next seqNum

            }
            // Remove the escape characters
            std::string msgWithoutEscStr = removeEsc(receivedMsg);
            // Send ACK (non self message, kind = 2)
            if (!isLost()){//Send ACK
                //EV <<"ACK" << R<<" is on the way" << endl;
                CustomMessage_Base* ackmsg = new CustomMessage_Base("");
                ackmsg->setAckNumber(R);
                ackmsg->setKind(2);//ACK (non self message, kind = 2)
                sendDelayed(ackmsg, PT+TD, "out");

                // print here sending ack with not lost
                print4(simTime()+PT, getIndex(), "ACK", R, "No");
            }else {
                print4(simTime()+PT, getIndex(), "ACK", R, "Yes");
            }
        }
        // Send NACK If corrupt
        else{
            //EV << "Receiver"<<" received message is corrupt" << endl;
            CustomMessage_Base* nackmsg = new CustomMessage_Base("");
            nackmsg->setAckNumber(R);
            nackmsg->setKind(3);//NACK (non self message, kind = 3)

            // print here sending nack
            print4(simTime()+PT , getIndex(), "NACK", R, "No");

            sendDelayed(nackmsg, PT+TD, "out");
        }
    }
    // Delete the message [Note: lost messages are not deleted here]
    if(!(RecivedMsg->isSelfMessage() && RecivedMsg->getKind()==1)){//The condition is to not delete the timer when it times out [it is deleted in send operation]
        cancelAndDelete(RecivedMsg);
    }
}
