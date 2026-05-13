// Member 1: Uzair Majeed 23i-3063 (Static Routing)
// Member 2: Muhammad Inam Ullah  23i-3058 (RIP)
// Member 3: Abdul Basit 23i-3018 (OSPF)

import inet.applications.contract.IApp;
simple TrafficGenerator like IApp
{
    parameters:


        string trafficProfile = default("CBR");

        // CBR Profile
        int    packetSize    = default(512);
        double sendInterval  = default(0.1);



        // Bursty Profile
        int    burstSize     = default(10);
        double burstInterval = default(0.01);
        double silenceTime   = default(1.0);



        string srcAddress    = default("0.0.0.0");
        string destAddress   = default("10.0.6.2");


        // DNS CONFIGURATION
        // ============================================================


        // DNS Configuration
        string dnsAddress    = default("10.0.5.2");
        bool   isDnsServer   = default(false);
        bool   isAppServer   = default(false);
        double startTime     = default(1.0);
        double stopTime      = default(175.0);



        // Module configuration
        bool isInet = default(false);
        @display("i=block/source");
        @class(TrafficGenerator);



        // Simulation signals


        @signal[packetsSent](type=long);
        @signal[packetsReceived](type=long);
        @signal[endToEndDelay](type=double);
        @signal[dnsQuerySent](type=long);
        @signal[dnsReplyRcvd](type=long);


        // Statistics


        @statistic[packetsSent](title="Packets Sent"; source=packetsSent; record=count,sum);
        @statistic[packetsReceived](title="Packets Received"; source=packetsReceived; record=count,sum);
        @statistic[endToEndDelay](title="End-to-End Delay"; source=endToEndDelay; record=mean,max,vector);
        @statistic[dnsQuerySent](title="DNS Queries Sent"; source=dnsQuerySent; record=count);
        @statistic[dnsReplyRcvd](title="DNS Replies Received"; source=dnsReplyRcvd; record=count);


    gates:
        inout ethg[] @loose;     // Used in Static Mode
        input socketIn @loose;   // Used in OSPF/RIP Mode
        output socketOut @loose; // Used in OSPF/RIP Mode

}
