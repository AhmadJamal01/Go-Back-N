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

#include "coordinator.h"

Define_Module(Coordinator);

void Coordinator::initialize()
{
    // TEMP (Data should be read from a file)
    // Send a message to the first node with its starting time
    double startTime = exponential(1/0.4);
    cMessage *msg = new cMessage(std::to_string(startTime).c_str(),0);
    send(msg, "out", 0);
}

void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
