// Member 1: Uzair Majeed 23i-3063 SE-B

#include "StaticRouter.h"
#include <sstream>
#include <algorithm>

// Tells OMNeT++ that this C++ class exists and is ready to use
Define_Module(StaticRouter);


// 1. Initialize: Sets up the router when the simulation starts
void StaticRouter::initialize()
{
    // Load parameters from configuration
    staticIP             = par("ipAddress").stdstringValue();
    totalPorts           = par("totalPorts");
    
    forwardedPacketCount = 0;
    droppedPacketCount   = 0;
    isLinkBroken         = false;

    // Register simulation signals for statistics
    packetsForwarded   = registerSignal("packetsForwarded");
    packetsDropped     = registerSignal("packetsDropped");
    perLinkUtilization = registerSignal("perLinkUtilization");

    loadRoutingTable();

    // Link failure/recovery simulation (Core or SE routers)
    if (strcmp(getName(), "coreRouter_S") == 0 || strcmp(getName(), "seRouter_S") == 0) {
        failureTimer  = new cMessage("LinkFailureAt60s");
        recoveryTimer = new cMessage("LinkRecoveryAt120s");
        scheduleAt(60.0,  failureTimer);
        scheduleAt(120.0, recoveryTimer);
    }
}



// 2. Load Routing Table: Parses routing rules from the configuration
void StaticRouter::loadRoutingTable()
{
    // Get the routing table parameter as a string
    std::string tableParam = par("routeTable").stdstringValue();

    // Remove whitespace
    tableParam.erase(std::remove(tableParam.begin(), tableParam.end(), ' '), tableParam.end());

    std::stringstream ss(tableParam);
    std::string entry;

    // Parse rules separated by semicolons (e.g., "10.0.6.0/24:2")
    while (std::getline(ss, entry, ';')) {
        if (entry.empty()) continue;

        // Extract IP/prefix and output port
        size_t colonPos  = entry.find(':');
        std::string networkPart = entry.substr(0, colonPos);     // e.g., "10.0.6.0/24"
        int outPort      = std::stoi(entry.substr(colonPos + 1)); // e.g., "2"

        // Split network address and prefix length
        size_t slashPos  = networkPart.find('/');
        std::string netAddress = networkPart.substr(0, slashPos);     // e.g., "10.0.6.0"
        int prefixLength = std::stoi(networkPart.substr(slashPos + 1)); // e.g., "24"

        // Populate routing table entry
        RouteEntry r;
        r.destinationNetwork = ipToInt(netAddress);
        r.prefixLength       = prefixLength;
        r.subnetMask         = prefixToMask(prefixLength);
        r.outPort            = outPort;
        
        forwardingTable.push_back(r);
    }

    EV << "[" << getName() << "] Loaded " << forwardingTable.size() << " routes.\n";
}



// 3. Handle Message: Main message processing engine
void StaticRouter::handleMessage(cMessage *msg)
{
    // Part A: Handle internal self-messages (alarms/timers)
    if (msg->isSelfMessage()) {

        // Link failure simulation
        if (strcmp(msg->getName(), "LinkFailureAt60s") == 0) {
            isLinkBroken = true;
            EV << "[" << getName() << "] LINK FAILURE at t=" << simTime() << "\n";
            delete msg;
        }
        // Link recovery simulation
        else if (strcmp(msg->getName(), "LinkRecoveryAt120s") == 0) {
            isLinkBroken = false;
            EV << "[" << getName() << "] LINK RECOVERED at t=" << simTime() << "\n";
            delete msg;
        }
        else {
            // Queue processing trigger
            processQueues();
            delete msg;
        }
        return;
    }

    // Part B: Handle incoming packets
    cPacket *pkt = check_and_cast<cPacket *>(msg);
    std::string pktName = pkt->getName();

    std::string sourceIPStr      = "0.0.0.0";
    std::string destinationIPStr = "0.0.0.0";

    // Extract source and destination IPs from packet name (Format: "DATA:srcIP:destIP:timestamp")
    size_t p1 = pktName.find(':');
    if (p1 != std::string::npos) {
        size_t p2 = pktName.find(':', p1 + 1);
        if (p2 != std::string::npos) {
            sourceIPStr = pktName.substr(p1 + 1, p2 - p1 - 1);
            size_t p3 = pktName.find(':', p2 + 1);
            destinationIPStr = pktName.substr(p2 + 1, (p3 != std::string::npos) ? p3 - p2 - 1 : std::string::npos);
        } else {
            destinationIPStr = pktName.substr(p1 + 1);
        }
    }

    // Routing decision via longest prefix match
    uint32_t destinationIP = ipToInt(destinationIPStr);
    int outPort = longestPrefixMatch(destinationIP);

    EV << "[" << getName() << "] Packet '" << pktName
       << "' -> dest=" << destinationIPStr << " -> port=" << outPort << "\n";

    // Part C: Packet forwarding or dropping
    // Drop if no route exists or if the link is broken on port 0
    if (outPort == -1 || (isLinkBroken && outPort == 0)) {
        EV_WARN << "[" << getName() << "] DROP src=" << sourceIPStr
                << " dst=" << destinationIPStr
                << " t=" << simTime()
                << " (linkBroken=" << isLinkBroken << " outPort=" << outPort << ")\n";

        emit(packetsDropped, ++droppedPacketCount);
        delete pkt;
        return;
    }

    // Enqueue packet for output port
    portQueues[outPort].push(pkt);

    cGate    *outGate  = gate("ethg$o", outPort);
    cChannel *channel  = outGate->getTransmissionChannel();

    // If channel is idle, trigger queue processing
    if (channel && channel->getTransmissionFinishTime() <= simTime()) {
        processQueues();
    }
}



// 4. Process Queues: Transmits packets from output queues to the channel
void StaticRouter::processQueues()
{
    for (int i = 0; i < totalPorts; i++) {
        
        if (!portQueues[i].empty()) {
            cGate    *outGate = gate("ethg$o", i);
            cChannel *channel = outGate->getTransmissionChannel();

            // Check if the transmission channel is free
            if (channel && channel->getTransmissionFinishTime() <= simTime()) {

                cPacket *sendPacket = portQueues[i].front();
                portQueues[i].pop();

                send(sendPacket, outGate);
                
                forwardedPacketCount++;
                emit(packetsForwarded, forwardedPacketCount);
                emit(perLinkUtilization, 1.0);

                // Schedule next check for when current transmission finishes
                scheduleAt(channel->getTransmissionFinishTime(), new cMessage("checkQueue"));
            }
        }
    }
}



// 5. Longest Prefix Match: Routing decision logic
int StaticRouter::longestPrefixMatch(uint32_t destinationIP)
{
    int bestPort      = -1;
    int bestPrefixLen = -1;

    for (const auto& route : forwardingTable) {
        
        // Match destination IP against route network using subnet mask
        if ((destinationIP & route.subnetMask) == (route.destinationNetwork & route.subnetMask)) {
            
            // Prefer the most specific match (longest prefix)
            if (route.prefixLength > bestPrefixLen) {
                bestPrefixLen = route.prefixLength;
                bestPort      = route.outPort;
            }
        }
    }
    return bestPort;
}



// 6. Math Helpers: IP and prefix conversion
uint32_t StaticRouter::ipToInt(const std::string& IP)
{
    uint32_t result = 0;
    std::stringstream ss(IP);
    std::string octet;
    int shift = 24;
    
    while (std::getline(ss, octet, '.') && shift >= 0) {
        try { 
            result |= (uint32_t(std::stoi(octet)) << shift); 
        }
        catch (...) { result |= 0; }
        shift -= 8;
    }
    return result;
}

uint32_t StaticRouter::prefixToMask(int prefix)
{
    return (prefix == 0) ? 0 : (~0u << (32 - prefix));
}



// 7. Finish: Record simulation results
void StaticRouter::finish()
{
    recordScalar("Final Packets Forwarded", forwardedPacketCount);
    recordScalar("Final Packets Dropped",   droppedPacketCount);
}