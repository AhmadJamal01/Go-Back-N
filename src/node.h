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

#ifndef __DATA_LINK_PROJECT_NODE_H_
#define __DATA_LINK_PROJECT_NODE_H_

#include <omnetpp.h>
#include "CustomMessage_m.h"

using namespace omnetpp;

/**
 * TODO - Generated class
 */
struct msgWithCommand{
  std::string payload="";
  std::string command="";
};
class Node : public cSimpleModule
{
  protected:
    // Byte stuffing constants
    char FLAG = '$';
    char ESCAPE = '/';
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void getCurrentNodeState(CustomMessage_Base *msg);
    void addParity (CustomMessage_Base* msg);
    bool checkParity (CustomMessage_Base* msg);
    std::string byteStuffing(std::string message);
    int WS = 5;
    double TO = 10.0;
    double PT = 0.5;
    double TD = 1.0;
    double ED = 4.0;
    double DD = 0.1;
    double LP = 0.1;
    omnetpp::SimTime EndProcessingTime=simTime();//drop the message if the node is still processing
    float msgDelay(std::string command);//-1 don't send loss
    float msgDubDelay(std::string command);//-1 don't send loss or no delay
    float processDelay();//just return PT
    float timeOutDelay();//just return TO
    bool isLost();//return true if ACK or NACK is lost for receiver
    int dist(int x, int y);
    int incrementSeqNum(int seqNum);
    CustomMessage_Base *timeoutMsgPtr = nullptr;
    unsigned int R = 0;
    unsigned int S = 0;
    unsigned int S_start = 0;
    unsigned int S_end = 0;
    unsigned int ackExpected = 0;
    msgWithCommand* msgBuffer = new msgWithCommand[WS];
    FILE *infile;
};
#endif
