// Member 1: Uzair Majeed 23i-3063 SE-B

simple StaticRouter 
{
    parameters:

        int    totalPorts  = default(4);
        string ipAddress   = default("0.0.0.0");
        string routeTable  = default("");

        // Simulation signals

        @signal[packetsForwarded](type=long);
        @signal[packetsDropped](type=long);

        @signal[perLinkUtilization](type=double);

        // Statistics

        @statistic[packetsForwarded](title="Packets Forwarded";  source=packetsForwarded;  record=count,sum);
        @statistic[packetsDropped](title="Packets Dropped";      source=packetsDropped;     record=count,sum);
        @statistic[perLinkUtilization](title="Link Utilization"; source=perLinkUtilization; record=vector,mean);

    gates:
        inout ethg[];
}
