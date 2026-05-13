// Member 1: Uzair Majeed 23i-3063 (Static Routing)
// Member 2: Muhammad Inam Ullah  23i-3058 (RIP)
// Member 3: Abdul Basit 23i-3018 (OSPF)

#include "TrafficGenerator.h"
#include <omnetpp.h>
#include <string>

#include "inet/common/packet/Packet.h"
#include "inet/common/Protocol.h"
#include "inet/common/packet/chunk/BytesChunk.h"
#include "inet/networklayer/common/L3Address.h"

using namespace omnetpp;


// TrafficGenerator: Registers with OMNeT++
Define_Module(TrafficGenerator);

void TrafficGenerator::initialize(int stage)
{
    // Stage 0: Local initialization (parameters and signals)
    if (stage == inet::INITSTAGE_LOCAL) {

        isInet = par("isInet").boolValue();
        trafficProfile = par("trafficProfile").stdstringValue();
        packetSize     = par("packetSize");
        sendInterval   = par("sendInterval");
        burstSize      = par("burstSize");
        burstInterval  = par("burstInterval");
        silenceTime    = par("silenceTime");
        destAddress    = par("destAddress").stdstringValue();
        srcAddress     = par("srcAddress").stdstringValue();
        startTime      = par("startTime");
        stopTime       = par("stopTime");
        dnsAddress     = par("dnsAddress").stdstringValue();
        isDnsServer    = par("isDnsServer");
        isAppServer    = par("isAppServer");


        seqNumber      = 0;
        burstCount     = 0;
        dnsResolved    = false;
        resolvedDestAddress = destAddress;   // Fallback for non-DNS mode


        // Register simulation signals
        packetsSentSignal     = registerSignal("packetsSent");
        packetsReceivedSignal = registerSignal("packetsReceived");
        endToEndDelaySignal   = registerSignal("endToEndDelay");
        dnsQuerySentSignal  = registerSignal("dnsQuerySent");
        dnsReplyRcvdSignal  = registerSignal("dnsReplyRcvd");



        if (isAppServer || isDnsServer) {
            EV << "[" << getName() << "] Running in SERVER mode. Waiting for packets.\n";
            return; 
        }


        sendTimer = new cMessage("SEND_TIMER");

        // Schedule initial DNS query or start data transmission
        if (!dnsAddress.empty() && dnsAddress != "0.0.0.0") {

            scheduleAt(startTime, new cMessage("SEND_DNS_QUERY"));
        } else {
            // Otherwise, skip DNS and just start sending data
            dnsResolved = true;
            scheduleAt(startTime, sendTimer);
        }
    }

    // Stage 3: Application layer initialization (socket binding)
    else if (stage == inet::INITSTAGE_APPLICATION_LAYER) {


        if (isInet) {
            socket.setCallback(this);
            socket.setOutputGate(gate("socketOut"));
            socket.bind(8000);
        }
    }
}

// Main message handler
void TrafficGenerator::handleMessage(cMessage *msg)
{
    // 1. Internal self-messages
    if (msg->isSelfMessage()) {
        if (strcmp(msg->getName(), "SEND_DNS_QUERY") == 0) {
            sendDNSQuery();
            delete msg;
            return;
        }
        if (msg == sendTimer) {
            if (simTime() > stopTime) {
                EV << "[" << getName() << "] Stop time reached. Done sending.\n";
                return;
            }
            sendPacket();
            scheduleNextSend();
            return;
        }
        delete msg;
        return;
    }



    // 2. INET mode processing
    if (isInet) {
        socket.processMessage(msg);
        return;
    }

    // 3. Static mode processing (raw packets)
    cPacket *pkt = check_and_cast<cPacket *>(msg);
    std::string pktName = pkt->getName();

    // Case A: DNS Query (acting as DNS Server)
    if (isDnsServer && pktName.size() >= 9 && pktName.substr(0, 9) == "DNS_QUERY") {
        handleDNSQuery(pkt);
        return;
    }

    // Case B: DNS Reply (received resolved IP)
    if (pktName.size() >= 9 && pktName.substr(0, 9) == "DNS_REPLY") {
        size_t c1 = pktName.find(':');
        size_t c2 = pktName.find(':', c1 + 1);
        if (c1 != std::string::npos && c2 != std::string::npos)
            resolvedDestAddress = pktName.substr(c1 + 1, c2 - c1 - 1);
        dnsResolved = true;
        emit(dnsReplyRcvdSignal, 1L);
        EV << "[" << getName() << "] DNS resolved! App server = " << resolvedDestAddress << "\n";
        delete pkt;
        // Schedule data transmission
        scheduleAt(simTime() + 0.01, sendTimer);
        return;
    }

    // Case C: Data packet (received at App Server)
    if (pktName.size() >= 5 && pktName.substr(0, 5) == "DATA:") {
        // Extract timestamp and calculate end-to-end delay
        size_t p1 = pktName.find(':');
        size_t p2 = pktName.find(':', p1 + 1);
        size_t p3 = pktName.find(':', p2 + 1);
        size_t p4 = pktName.find(':', p3 + 1);
        if (p3 != std::string::npos) {
            double sendTime = std::stod(pktName.substr(p3 + 1,
                (p4 != std::string::npos) ? p4 - p3 - 1 : std::string::npos));
            double delay = simTime().dbl() - sendTime;
            emit(endToEndDelaySignal, delay);
            emit(packetsReceivedSignal, 1L);
            EV << "[AppServer] Packet received. Delay=" << delay << "s\n";
        }
        delete pkt;
        return;
    }

    // If it's none of the above, it's junk. Drop it.
    EV_WARN << "[" << getName() << "] Unknown packet: " << pktName << " — dropping.\n";
    delete pkt;
}


// INET UDP socket callback
void TrafficGenerator::socketDataArrived(inet::UdpSocket *sock, inet::Packet *packet)
{
    // Logic matches static mode but uses INET packets
    std::string pktName = packet->getName();


    if (isDnsServer && pktName.size() >= 9 && pktName.substr(0, 9) == "DNS_QUERY") {
        handleDNSQuery(packet);   // handleDNSQuery deletes the packet internally
        return;
    }

    if (pktName.size() >= 9 && pktName.substr(0, 9) == "DNS_REPLY") {
        size_t c1 = pktName.find(':');
        size_t c2 = pktName.find(':', c1 + 1);
        if (c1 != std::string::npos && c2 != std::string::npos)
            resolvedDestAddress = pktName.substr(c1 + 1, c2 - c1 - 1);
        dnsResolved = true;
        emit(dnsReplyRcvdSignal, 1L);
        EV << "[" << getName() << "] DNS resolved! App server = " << resolvedDestAddress << "\n";
        delete packet;
        scheduleAt(simTime() + 0.01, sendTimer);
        return;
    }

    if (pktName.size() >= 5 && pktName.substr(0, 5) == "DATA:") {
        size_t p1 = pktName.find(':');
        size_t p2 = pktName.find(':', p1 + 1);
        size_t p3 = pktName.find(':', p2 + 1);
        size_t p4 = pktName.find(':', p3 + 1);
        if (p3 != std::string::npos) {
            double sendTime = std::stod(pktName.substr(p3 + 1,
                (p4 != std::string::npos) ? p4 - p3 - 1 : std::string::npos));
            double delay = simTime().dbl() - sendTime;
            emit(endToEndDelaySignal, delay);
            emit(packetsReceivedSignal, 1L);
            EV_INFO << "[AppServer] INET packet received. Delay=" << delay << "s\n";
        }
        delete packet;
        return;
    }

    EV_WARN << "[" << getName() << "] INET: unknown packet '" << pktName << "' — dropping.\n";
    delete packet;
}

// INET socket error callback
void TrafficGenerator::socketErrorArrived(inet::UdpSocket *sock, inet::Indication *indication)
{
    EV_WARN << "[" << getName() << "] UDP socket error — ignoring.\n";
    delete indication;
}

// Sends a DNS query to resolve the destination address
void TrafficGenerator::sendDNSQuery()
{
    std::string queryName = "DNS_QUERY:" + srcAddress;

    emit(dnsQuerySentSignal, 1L);
    EV << "[" << getName() << "] Sending DNS query to " << dnsAddress << "\n";

    // Format: "DNS_QUERY:srcIP:dnsIP:timestamp"
    std::string routedName = "DNS_QUERY:" + srcAddress + ":" + dnsAddress + ":" + std::to_string(simTime().dbl());

    if (isInet) {
        // INET Mode: UDP packet with 64B payload
        inet::Packet *inetPkt = new inet::Packet(routedName.c_str());
        const auto& payload = inet::makeShared<inet::BytesChunk>();
        std::vector<uint8_t> bytes(64, 0);
        payload->setBytes(bytes);
        inetPkt->insertAtBack(payload);

        inet::L3Address dest(dnsAddress.c_str());
        socket.sendTo(inetPkt, dest, 8000);
    } else {
        // Static Mode: raw cPacket
        cPacket *query = new cPacket(queryName.c_str());
        query->setByteLength(64);
        query->setName(routedName.c_str());
        send(query, "ethg$o", 0);
    }
}


// DNS Server: Handles incoming DNS queries and sends replies
void TrafficGenerator::handleDNSQuery(cPacket *query)
{
    std::string name = query->getName();
    size_t p1 = name.find(':');
    size_t p2 = name.find(':', p1 + 1);
    std::string requesterIP = name.substr(p1 + 1, p2 - p1 - 1);

    EV << "[DNS Server] Received query from " << requesterIP << ". Replying with appServer IP.\n";

    // Reply format: "DNS_REPLY:appServerIP:srcIP:timestamp"
    std::string replyName = "DNS_REPLY:10.0.6.2:" + requesterIP + ":" + std::to_string(simTime().dbl());

    delete query;

    if (isInet) {
        // INET Mode: UDP reply
        inet::Packet *inetPkt = new inet::Packet(replyName.c_str());
        const auto& payload = inet::makeShared<inet::BytesChunk>();
        std::vector<uint8_t> bytes(64, 0);
        payload->setBytes(bytes);
        inetPkt->insertAtBack(payload);

        inet::L3Address dest(requesterIP.c_str());
        socket.sendTo(inetPkt, dest, 8000);
    } else {
        // Static Mode: raw packet reply
        cPacket *reply = new cPacket(replyName.c_str());
        reply->setByteLength(64);
        send(reply, "ethg$o", 0);
    }
}


// Transmits a data packet
void TrafficGenerator::sendPacket()
{
    seqNumber++;

    // Packet name format: "DATA:srcIP:destIP:timestamp:seqNum"
    std::string pktName = "DATA:" + srcAddress + ":"
                        + resolvedDestAddress + ":"
                        + std::to_string(simTime().dbl()) + ":"
                        + std::to_string(seqNumber);



    if (isInet) {
        // INET Mode: UDP packet padded to packetSize
        inet::Packet *inetPkt = new inet::Packet(pktName.c_str());
        const auto& payload = inet::makeShared<inet::BytesChunk>();
        std::vector<uint8_t> bytes(packetSize, 0);
        payload->setBytes(bytes);
        inetPkt->insertAtBack(payload);

        inet::L3Address dest(resolvedDestAddress.c_str());
        socket.sendTo(inetPkt, dest, 8000);
    }
    else {
        // Static Mode: raw cPacket
        cPacket *pkt = new cPacket(pktName.c_str());
        pkt->setByteLength(packetSize);
        send(pkt, "ethg$o", 0);
    }

    emit(packetsSentSignal, 1L);


    EV_INFO << "[" << getName() << "] Sent pkt #" << seqNumber
            << " src=" << srcAddress
            << " dst=" << resolvedDestAddress
            << " size=" << packetSize << "B"
            << " t=" << simTime() << "\n";
}

// Schedules the next transmission based on the traffic profile
void TrafficGenerator::scheduleNextSend()
{
    if (trafficProfile == "CBR") {
        // Constant Bit Rate: fixed interval
        scheduleAt(simTime() + sendInterval, sendTimer);
    }
    else if (trafficProfile == "Bursty") {
        // Bursty: high-rate bursts followed by silence
        burstCount++;
        if (burstCount < burstSize) {
            scheduleAt(simTime() + burstInterval, sendTimer);
        } else {
            burstCount = 0;
            scheduleAt(simTime() + silenceTime, sendTimer);
        }
    }
    else {
        EV_WARN << "Unknown traffic profile: " << trafficProfile << "\n";
    }
}



// Simulation finalization
void TrafficGenerator::finish()
{
    recordScalar("TotalPacketsSent", seqNumber);
}

