// Member 1: Uzair Majeed 23i-3063 SE-B

#ifndef STATICROUTER_H
#define STATICROUTER_H

#include <omnetpp.h>
#include <map>
#include <queue>
#include <vector>
#include <string>


using namespace omnetpp;



// RouteEntry: Structure for each routing table entry
struct RouteEntry {
    uint32_t destinationNetwork;
    uint32_t subnetMask;
    int prefixLength;
    int outPort;
};

// StaticRouter: Simple module for static IP routing
class StaticRouter : public cSimpleModule

{
  protected:



    std::string staticIP;
    int         totalPorts;

    std::vector<RouteEntry> forwardingTable;
    bool isLinkBroken;



    // Statistics signals
    simsignal_t packetsForwarded;
    simsignal_t packetsDropped;
    simsignal_t perLinkUtilization;


    long forwardedPacketCount;
    long droppedPacketCount;

    // Timers

    // Timers for link failure/recovery simulation
    cMessage *failureTimer;
    cMessage *recoveryTimer;



    // Output queues mapped by port number
    std::map<int, std::queue<cPacket*>> portQueues;



    void processQueues();

    virtual void initialize() override;
    virtual void handleMessage(cMessage *) override;
    virtual void finish() override;

    void loadRoutingTable();

    // Routing decision via longest prefix match
    int longestPrefixMatch(uint32_t destinationIP);



    // IP and subnet mask conversion helpers
    uint32_t ipToInt(const std::string& IP);
    uint32_t prefixToMask(int prefix);

};

#endif
