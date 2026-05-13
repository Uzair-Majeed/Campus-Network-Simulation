// Member 1: Uzair Majeed 23i-3063 (Static Routing)
// Member 2: Muhammad Inam Ullah  23i-3058 (RIP)
// Member 3: Abdul Basit 23i-3018 (OSPF)

// Campus Network Topology
// Mixed configuration for Static, RIP, and OSPF routing

network CampusNetwork
{
    parameters:
        @display("bgb=1000,800");

        bool   isInet      = default(false);                        // false=Static, true=RIP/OSPF
        string hostType    = default("inet.node.inet.StandardHost");
        string routerType  = default("inet.node.ospfv2.OspfRouter");


    types:
        // Backbone: 100Mbps, 2ms delay, 0.1% PER
        channel BackboneLink extends ned.DatarateChannel { datarate = 100Mbps; delay = 2ms; per = 0.001; }
        // Access: 10Mbps, 1ms delay
        channel AccessLink extends ned.DatarateChannel { datarate = 10Mbps; delay = 1ms; }
        // Server: 100Mbps, 1ms delay
        channel ServerLink extends ned.DatarateChannel { datarate = 100Mbps; delay = 1ms; }


    submodules:
        // INET support modules (active when isInet=true)


        // Assigns IP addresses from config XML — disabled for static routing
        configurator: inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator if isInet {
            @display("p=100,50;is=s");
        }

        // Logs every route add/delete/change to .elog
        // Used to measure convergence time (Requirement 2 for RIP and OSPF)
        routingTableRecorder: inet.networklayer.ipv4.RoutingTableRecorder if isInet {
            @display("p=100,100;is=s");
        }

        // Runs timed failure events from scenario_inet.xml
        // SE-Core link down at t=60s, reconnect at t=120s

        scenarioManager: inet.common.scenario.ScenarioManager if isInet {
            @display("p=100,150;is=s");
        }

        // Static routing modules (active when isInet=false)


        coreRouter_S:  StaticRouter if !isInet { @display("p=500,400;i=abstract/router;t=Core Router\n10.0.0.1"); }
        seRouter_S:    StaticRouter if !isInet { @display("p=200,200;i=abstract/router;t=SE Router\n10.0.1.2"); }
        eeRouter_S:    StaticRouter if !isInet { @display("p=800,200;i=abstract/router;t=EE Router\n10.0.2.2"); }
        bbaRouter_S:   StaticRouter if !isInet { @display("p=200,600;i=abstract/router;t=BBA Router\n10.0.3.2"); }
        adminRouter_S: StaticRouter if !isInet { @display("p=800,600;i=abstract/router;t=Admin Router\n10.0.4.2"); }


        seHost1_S:    TrafficGenerator if !isInet { @display("p=50,100;i=device/pc;t=SE Host 1\n192.168.1.2"); }
        seHost2_S:    TrafficGenerator if !isInet { @display("p=150,100;i=device/pc;t=SE Host 2\n192.168.1.3"); }
        seHost3_S:    TrafficGenerator if !isInet { @display("p=250,100;i=device/pc;t=SE Host 3\n192.168.1.4"); }

        eeHost1_S:    TrafficGenerator if !isInet { @display("p=700,100;i=device/pc;t=EE Host 1\n192.168.2.2"); }
        eeHost2_S:    TrafficGenerator if !isInet { @display("p=800,100;i=device/pc;t=EE Host 2\n192.168.2.3"); }
        eeHost3_S:    TrafficGenerator if !isInet { @display("p=900,100;i=device/pc;t=EE Host 3\n192.168.2.4"); }

        bbaHost1_S:   TrafficGenerator if !isInet { @display("p=50,700;i=device/pc;t=BBA Host 1\n192.168.3.2"); }
        bbaHost2_S:   TrafficGenerator if !isInet { @display("p=150,700;i=device/pc;t=BBA Host 2\n192.168.3.3"); }
        bbaHost3_S:   TrafficGenerator if !isInet { @display("p=250,700;i=device/pc;t=BBA Host 3\n192.168.3.4"); }

        adminHost1_S: TrafficGenerator if !isInet { @display("p=700,700;i=device/pc;t=Admin Host 1\n192.168.4.2"); }
        adminHost2_S: TrafficGenerator if !isInet { @display("p=800,700;i=device/pc;t=Admin Host 2\n192.168.4.3"); }
        adminHost3_S: TrafficGenerator if !isInet { @display("p=900,700;i=device/pc;t=Admin Host 3\n192.168.4.4"); }


        dnsServer_S:  TrafficGenerator if !isInet { @display("p=400,500;i=device/server;t=DNS Server\n10.0.5.2"); }
        appServer_S:  TrafficGenerator if !isInet { @display("p=600,500;i=device/server;t=App Server\n10.0.6.2"); }


        // INET modules (active when isInet=true)


        coreRouter_I:  inet.node.inet.Router if isInet { @display("p=500,400;i=abstract/router;t=Core Router\nArea 0"); }
        seRouter_I:    inet.node.inet.Router if isInet { @display("p=200,200;i=abstract/router;t=SE Router\nSubnet .1.x"); }
        eeRouter_I:    inet.node.inet.Router if isInet { @display("p=800,200;i=abstract/router;t=EE Router\nSubnet .2.x"); }
        bbaRouter_I:   inet.node.inet.Router if isInet { @display("p=200,600;i=abstract/router;t=BBA Router\nSubnet .3.x"); }
        adminRouter_I: inet.node.inet.Router if isInet { @display("p=800,600;i=abstract/router;t=Admin Router\nSubnet .4.x"); }


        seHost1_I:    inet.node.inet.StandardHost if isInet { @display("p=50,100;i=device/pc;t=SE Host 1\nAuto IP"); }
        seHost2_I:    inet.node.inet.StandardHost if isInet { @display("p=150,100;i=device/pc;t=SE Host 2\nAuto IP"); }
        seHost3_I:    inet.node.inet.StandardHost if isInet { @display("p=250,100;i=device/pc;t=SE Host 3\nAuto IP"); }
        eeHost1_I:    inet.node.inet.StandardHost if isInet { @display("p=700,100;i=device/pc;t=EE Host 1\nAuto IP"); }
        eeHost2_I:    inet.node.inet.StandardHost if isInet { @display("p=800,100;i=device/pc;t=EE Host 2\nAuto IP"); }
        eeHost3_I:    inet.node.inet.StandardHost if isInet { @display("p=900,100;i=device/pc;t=EE Host 3\nAuto IP"); }
        bbaHost1_I:   inet.node.inet.StandardHost if isInet { @display("p=50,700;i=device/pc;t=BBA Host 1\nAuto IP"); }
        bbaHost2_I:   inet.node.inet.StandardHost if isInet { @display("p=150,700;i=device/pc;t=BBA Host 2\nAuto IP"); }
        bbaHost3_I:   inet.node.inet.StandardHost if isInet { @display("p=250,700;i=device/pc;t=BBA Host 3\nAuto IP"); }
        adminHost1_I: inet.node.inet.StandardHost if isInet { @display("p=700,700;i=device/pc;t=Admin Host 1\nAuto IP"); }
        adminHost2_I: inet.node.inet.StandardHost if isInet { @display("p=800,700;i=device/pc;t=Admin Host 2\nAuto IP"); }
        adminHost3_I: inet.node.inet.StandardHost if isInet { @display("p=900,700;i=device/pc;t=Admin Host 3\nAuto IP"); }

        dnsServer_I:  inet.node.inet.StandardHost if isInet { @display("p=400,500;i=device/server;t=DNS Server\n10.0.5.2"); }
        appServer_I:  inet.node.inet.StandardHost if isInet { @display("p=600,500;i=device/server;t=App Server\n10.0.6.2"); }



        connections allowunconnected:

        // Static connections

        coreRouter_S.ethg++ <--> BackboneLink { @display("m=100Mbps,2ms"); } <--> seRouter_S.ethg++    if !isInet;
        coreRouter_S.ethg++ <--> BackboneLink { @display("m=100Mbps,2ms"); } <--> eeRouter_S.ethg++    if !isInet;
        coreRouter_S.ethg++ <--> BackboneLink { @display("m=100Mbps,2ms"); } <--> bbaRouter_S.ethg++   if !isInet;
        coreRouter_S.ethg++ <--> BackboneLink { @display("m=100Mbps,2ms"); } <--> adminRouter_S.ethg++ if !isInet;
        coreRouter_S.ethg++ <--> ServerLink   { @display("m=100Mbps,1ms"); } <--> dnsServer_S.ethg++   if !isInet;
        coreRouter_S.ethg++ <--> ServerLink   { @display("m=100Mbps,1ms"); } <--> appServer_S.ethg++   if !isInet;


        seRouter_S.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> seHost1_S.ethg++     if !isInet;
        seRouter_S.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> seHost2_S.ethg++     if !isInet;
        seRouter_S.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> seHost3_S.ethg++     if !isInet;
        eeRouter_S.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> eeHost1_S.ethg++     if !isInet;
        eeRouter_S.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> eeHost2_S.ethg++     if !isInet;
        eeRouter_S.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> eeHost3_S.ethg++     if !isInet;
        bbaRouter_S.ethg++   <--> AccessLink { @display("m=10Mbps,1ms"); } <--> bbaHost1_S.ethg++    if !isInet;
        bbaRouter_S.ethg++   <--> AccessLink { @display("m=10Mbps,1ms"); } <--> bbaHost2_S.ethg++    if !isInet;
        bbaRouter_S.ethg++   <--> AccessLink { @display("m=10Mbps,1ms"); } <--> bbaHost3_S.ethg++    if !isInet;
        adminRouter_S.ethg++ <--> AccessLink { @display("m=10Mbps,1ms"); } <--> adminHost1_S.ethg++  if !isInet;
        adminRouter_S.ethg++ <--> AccessLink { @display("m=10Mbps,1ms"); } <--> adminHost2_S.ethg++  if !isInet;
        adminRouter_S.ethg++ <--> AccessLink { @display("m=10Mbps,1ms"); } <--> adminHost3_S.ethg++  if !isInet;



        // INET connections

        coreRouter_I.ethg++ <--> BackboneLink { @display("m=100Mbps,2ms"); } <--> seRouter_I.ethg++    if isInet;
        coreRouter_I.ethg++ <--> BackboneLink { @display("m=100Mbps,2ms"); } <--> eeRouter_I.ethg++    if isInet;
        coreRouter_I.ethg++ <--> BackboneLink { @display("m=100Mbps,2ms"); } <--> bbaRouter_I.ethg++   if isInet;
        coreRouter_I.ethg++ <--> BackboneLink { @display("m=100Mbps,2ms"); } <--> adminRouter_I.ethg++ if isInet;
        coreRouter_I.ethg++ <--> ServerLink   { @display("m=100Mbps,1ms"); } <--> dnsServer_I.ethg++   if isInet;
        coreRouter_I.ethg++ <--> ServerLink   { @display("m=100Mbps,1ms"); } <--> appServer_I.ethg++   if isInet;


        seRouter_I.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> seHost1_I.ethg++     if isInet;
        seRouter_I.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> seHost2_I.ethg++     if isInet;
        seRouter_I.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> seHost3_I.ethg++     if isInet;
        eeRouter_I.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> eeHost1_I.ethg++     if isInet;
        eeRouter_I.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> eeHost2_I.ethg++     if isInet;
        eeRouter_I.ethg++    <--> AccessLink { @display("m=10Mbps,1ms"); } <--> eeHost3_I.ethg++     if isInet;
        bbaRouter_I.ethg++   <--> AccessLink { @display("m=10Mbps,1ms"); } <--> bbaHost1_I.ethg++    if isInet;
        bbaRouter_I.ethg++   <--> AccessLink { @display("m=10Mbps,1ms"); } <--> bbaHost2_I.ethg++    if isInet;
        bbaRouter_I.ethg++   <--> AccessLink { @display("m=10Mbps,1ms"); } <--> bbaHost3_I.ethg++    if isInet;
        adminRouter_I.ethg++ <--> AccessLink { @display("m=10Mbps,1ms"); } <--> adminHost1_I.ethg++  if isInet;
        adminRouter_I.ethg++ <--> AccessLink { @display("m=10Mbps,1ms"); } <--> adminHost2_I.ethg++  if isInet;
        adminRouter_I.ethg++ <--> AccessLink { @display("m=10Mbps,1ms"); } <--> adminHost3_I.ethg++  if isInet;


}