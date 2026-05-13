// Member 1: Uzair Majeed 23i-3063 (Static Routing)
// Member 2: Muhammad Inam Ullah  23i-3058 (RIP)
// Member 3: Abdul Basit 23i-3018 (OSPF)


#ifndef TRAFFICGENERATOR_H
#define TRAFFICGENERATOR_H

#include <omnetpp.h>
#include <string>
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/InitStages.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/Message.h"

using namespace omnetpp;



// TrafficGenerator: Module for generating and receiving network traffic
class TrafficGenerator : public cSimpleModule, public inet::UdpSocket::ICallback

{
  private:

    // Configuration parameters
    std::string trafficProfile;
    int         packetSize;
    double      sendInterval;
    int         burstSize;
    double      burstInterval;
    double      silenceTime;
    std::string destAddress;
    std::string srcAddress;
    double      startTime;
    double      stopTime;


    // DNS and role configuration
    std::string dnsAddress;
    bool        isDnsServer;
    bool        isAppServer;




    long        seqNumber;
    int         burstCount;
    bool        dnsResolved;
    std::string resolvedDestAddress;


    // Dual-mode support (INET vs custom routing)
    bool isInet;
    inet::UdpSocket socket;
    
    cMessage   *sendTimer;



    // Statistics signals
    simsignal_t packetsSentSignal;
    simsignal_t packetsReceivedSignal;
    simsignal_t endToEndDelaySignal;
    simsignal_t dnsQuerySentSignal;
    simsignal_t dnsReplyRcvdSignal;



    void sendPacket();
    void scheduleNextSend();
    void sendDNSQuery();
    void handleDNSQuery(cPacket *query);



    // INET UDP callbacks
    virtual void socketDataArrived(inet::UdpSocket *socket, inet::Packet *packet) override;
    virtual void socketErrorArrived(inet::UdpSocket *socket, inet::Indication *indication) override;

    virtual void socketClosed(inet::UdpSocket *socket) override {}

  protected:
  

    // Core OMNeT++ functions
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;

    // Called at the very end of the simulation. Used for cleanup and final stat calculations.
    virtual void finish()                  override;
};

#endif
