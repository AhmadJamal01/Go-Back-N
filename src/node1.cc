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

#include "node1.h"

Define_Module(Node1);

void Node1::initialize()
{
    // TODO - Generated method body
}

void Node1::getCurrentNodeState(cMessage *msg)
{
    // Check if the received message came from coordinator or the another node
        // The coordinator is connected to input gate 0 of each node (According to package.ned file)
        if (msg->getArrivalGate()->getIndex() == 0)
        {
            EV << "Node 1 will act as sender.." << endl;
            EV << "Node 0 will act as receiver.." << endl;
            EV << "Node 1 will start after: " << msg->getName() << endl;
            double startTime = double(std::stod(msg->getName()));
            scheduleAt(simTime() + startTime, new cMessage(""));
        }
        else
        {
            // Handle sender or receiver case
            if (strcmp(msg->getName(), "ACK" )== 0)
            {

            }
            else if (strcmp(msg->getName(), "NACK") == 0)
            {
                // Will be ignored (see classroom)
            }
            else {
                // This node acts as receiver
                // Remove escape characters from the payload
                std::string receivedmsg = msg->getName();
                std::vector<char> msgWithoutEsc ;
                bool isPrevEsc = false;
                for (int i=1 ; i< receivedmsg.size()-1 ; i++)
                {
                    if (isPrevEsc)
                    {
                        msgWithoutEsc.push_back(receivedmsg[i]);
                        isPrevEsc = false ;
                        continue;
                    }
                    else if (!isPrevEsc && receivedmsg[i]==ESCAPE)
                    {
                        isPrevEsc = true;
                        continue;
                    }
                    msgWithoutEsc.push_back(receivedmsg[i]);

                }
                // Convert vector back to string
                std::string msgWithoutEscStr(msgWithoutEsc.begin(), msgWithoutEsc.end());
                EV << "Received after removing escape characters: "<< msgWithoutEscStr << endl;

            }
        }
}

void Node1::handleMessage(cMessage *msg)
{
    std::vector<char> stuffedmsg;
    if (msg->isSelfMessage())
    {
        EV << "Node 1 Started ..." << endl;
        // Handle flag and escape characters in the payload
        // And add the flag to the start and end of the message
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
        // Print the msg to console
        EV<< stuffedstr << endl;
        msg->setName(stuffedstr.c_str());
        send(msg,"out");
    }
    else
    {
        getCurrentNodeState(msg);
    }
}

