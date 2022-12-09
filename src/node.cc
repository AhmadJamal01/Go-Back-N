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

void Node::initialize()
{
    // TODO - Generated method body
}

void Node::getCurrentNodeState(CustomMessage_Base *msg)
{
    // Check if the received message came from coordinator or the another node
    // The message from coordinator will have kind = 0
    if (msg->getKind() == 0)
    {
        EV << "Node "<< getIndex() <<" will act as sender.." << endl;
        EV << "Node "<< 1-getIndex() <<" will act as receiver.." << endl;
        EV << "Node "<< getIndex() <<" will start after: " << msg->getPayload() << endl;
        double startTime = double(std::stod(msg->getPayload()));
        // Send a self message to start the node after the specified time
        scheduleAt(simTime() + startTime, new CustomMessage_Base(""));
    }
    else
    {
        // if the frame type is ACK
        if (msg->getFrameType() == 1)
        {

        }
        // if the frame type is NACK
        else if (msg->getFrameType() == 2)
        {
             // Will be ignored (see classroom)
        }
        else
        {
              // This node acts as receiver
              // Remove escape characters from the payload
              std::string receivedMsg = msg->getPayload();
              // Check the parity
              bool isNotCorrupt = checkParity(msg);
              EV << "Parity Check: "<< isNotCorrupt << endl;
              // If the message is not corrupt remove the escape characters
              if(isNotCorrupt){
                  std::vector<char> msgWithoutEsc ;
                  bool isPrevEsc = false;
                  for (int i=1 ; i< receivedMsg.size()-1 ; i++)
                  {
                      if (isPrevEsc)
                      {
                          msgWithoutEsc.push_back(receivedMsg[i]);
                          isPrevEsc = false ;
                          continue;
                      }
                      else if (!isPrevEsc && receivedMsg[i]==ESCAPE)
                      {
                          isPrevEsc = true;
                          continue;
                      }
                      msgWithoutEsc.push_back(receivedMsg[i]);

                  }
                  // Convert vector back to string
                  std::string msgWithoutEscStr(msgWithoutEsc.begin(), msgWithoutEsc.end());
                  EV << "Received after removing escape characters: "<< msgWithoutEscStr << endl;

                  // TODO:: Send ACK
              }
              // TODO:: Send NACK

        }
    }
}


void Node::handleMessage(cMessage *msg)
{
    // Upcasting
    CustomMessage_Base* mmsg = check_and_cast<CustomMessage_Base*>(msg);
    if (mmsg->isSelfMessage())
    {
        EV << "Node " << getIndex() <<" Started ..." << endl;
        // Apply Framing
        std::string stuffedstr = byteStuffing(MESSAGE);
        // Set the payload with the stuffed message
        mmsg -> setPayload(stuffedstr.c_str());
        // Add parity
        addParity(mmsg);
        // Print the msg to console
        EV<< "Node " << getIndex() << " sent " <<mmsg -> getPayload() << endl;
        EV<< mmsg -> getTrailer() << endl;
        // Set the kind to 1 to differentiate between the coordinator (Kind 0) and the other nodes
        mmsg->setKind(1);
        send(mmsg,"out");
    }
    else
    {
        // Handle if the received message is a self message or Coordinator message or a message from another node
        getCurrentNodeState(mmsg);
    }
}
