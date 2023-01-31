
# Go back N Protocol Simulation
## **Project Overview**
Implementation and simulation of Go back N protocol in C++ using omnetpp
## **Go back N Overview**
It is a data link layer protocol that is used to ensure reliable data transfer between two nodes.

It is from the sliding window protocols family.
## **Omnetpp**
omnet++ is a discrete event simulation framework for building network simulations.

It is used to simulate the network and the protocol.
## **Implementation Details**
### **System Architecture**
The system consists of two nodes and a coordinator.

The coordinator's job is to tell which node is the sender and which is the receiver.

The two nodes are designated, with one serving as the sender and the other as the receiver, where the sender exclusively transmits data and the receiver exclusively receives it.
### **Errors Simulation**
#### **Bit Error**
The system introduces errors in the data sent by the sender in the form of flipping one bit randomly in the data sent.
#### **Packet Loss**
The system introduces packet loss by dropping a packet randomly (that happens at the sendera and the receiver).
#### **Duplication**
The system introduces duplication by duplicating a packet randomly.
## **Screenshots**
![alt text](https://i.imgur.com/6LWuSDM.jpg)