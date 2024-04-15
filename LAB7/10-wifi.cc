#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/tcp-westwood.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

Ptr<PacketSink> sink;     /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0; /* The value of the last total received bytes */
void CalculateThroughput()
{
    Time now = Simulator::Now();                                       /* Return the simulator's virtual time. */
    double cur = (sink->GetTotalRx() - lastTotalRx) * (double)8 / 1e5; /* Convert Application RX
  Packets to MBits. */
    std::cout << now.GetSeconds() << "s: \t" << cur << " Mbit/s" << std::endl;
    lastTotalRx = sink->GetTotalRx();
    Simulator::Schedule(MilliSeconds(100), &CalculateThroughput);
}

int main(int argc, char *argv[])
{
    // uint32_t payloadSize = 1472; /* Transport layer payload size in bytes. */
    std::string dataRate = "100Mbps";      /* Application layer datarate. */
    std::string tcpVariant = "TcpNewReno"; /* TCP variant type. */
    std::string phyRate = "HtMcs7";        /* Physical layer bitrate. */
    double simulationTime = 10;            /* Simulation time in seconds. */
                                           // bool pcapTracing = false; /* PCAP Tracing is enabled or not. */

    // LogComponentEnable("YansWifiChannel", ns3::LOG_LEVEL_INFO);
    // LogComponentEnable("YansWifiPhy", ns3::LOG_LEVEL_INFO);
    // LogComponentEnable("OnOffApplication", ns3::LOG_LEVEL_INFO);

    NodeContainer wifiStaNode;
    wifiStaNode.Create(10); // Create 10 station node objects
    NodeContainer wifiApNode;
    wifiApNode.Create(1); // Create 1 access point node object

    //
    WifiHelper wifi;
    wifi.SetStandard(ns3::WIFI_STANDARD_80211n_2_4GHZ);

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(5e9));

    YansWifiPhyHelper wifiPhyHelper;
    wifiPhyHelper.SetChannel(wifiChannel.Create());
    wifiPhyHelper.SetErrorRateModel("ns3::YansErrorRateModel");

    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue(phyRate),
                                 "ControlMode", StringValue("HtMcs0"));

    // Install PHY and MAC
    Ssid ssid = Ssid("ns3-wifi");
    WifiMacHelper mac;

    mac.SetType("ns3::StaWifiMac","Ssid", SsidValue(ssid),"ActiveProbing", BooleanValue(false));

    NetDeviceContainer staDevices;
    staDevices = wifi.Install(wifiPhyHelper, mac, wifiStaNode);

    mac.SetType("ns3::ApWifiMac","Ssid", SsidValue(ssid));

    NetDeviceContainer apDevice;
    apDevice = wifi.Install(wifiPhyHelper, mac, wifiApNode);

    InternetStackHelper stack;
    stack.Install(wifiStaNode);
    stack.Install(wifiApNode);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");

    Ipv4InterfaceContainer apInterface;
    apInterface = address.Assign(apDevice);
    Ipv4InterfaceContainer staInterface;
    staInterface = address.Assign(staDevices);

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    positionAlloc->Add(Vector(1.0, 1.0, 0.0));



  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNode);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

    ApplicationContainer sourceApplications, sinkApplications;

    uint32_t portNumber = 9;
    for (uint32_t index = 0; index < 10; ++index)
    {
        auto ipv4 = wifiApNode.Get(0)->GetObject<Ipv4>();
        const auto address = ipv4->GetAddress(1, 0).GetLocal();
        InetSocketAddress sinkSocket(address, portNumber++);
        OnOffHelper onOffHelper("ns3::UdpSocketFactory", sinkSocket);
        // OnOffHelper onOffHelper("ns3::UdpSocketFactory", InetSocketAddress(staInterface.GetAddress(index),portNumber));
        onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        onOffHelper.SetAttribute("DataRate", DataRateValue(50000000/ 10));
        onOffHelper.SetAttribute("PacketSize", UintegerValue(1472)); // bytes
        sourceApplications.Add(onOffHelper.Install(wifiStaNode.Get(index)));
        PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", sinkSocket);
        sinkApplications.Add(packetSinkHelper.Install(wifiApNode.Get(0)));
        // PacketSinkHelper packetSinkHelper("ns3::UdpSocketFactory", InetSocketAddress(apInterface.GetAddress(0),portNumber++));
        // sinkApplications.Add(packetSinkHelper.Install(wifiApNode.Get(0)));
        sink = StaticCast<PacketSink>(sourceApplications.Get(0));
    }

    /* Install TCP Receiver on the access point */
    //    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny
    //                                                                                   (), 9));
    //    Ptr <PacketSink> sink;
    //    ApplicationContainer sinkApp = sinkHelper.Install(wifiApNode);
    //    sink = StaticCast<PacketSink>(sinkApp.Get(0));
    ///* Install TCP/UDP Transmitter on the station */
    //    OnOffHelper server("ns3::TcpSocketFactory", (InetSocketAddress(apInterface.GetAddress(0),
    //                                                                   9)));
    //    server.SetAttribute("PacketSize", UintegerValue(payloadSize));
    //    server.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    //    server.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    //    server.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
    //    ApplicationContainer serverApp = server.Install(wifiStaNode);
    /* Start Applications */
    
    sinkApplications.Start(Seconds(0.0));
    sourceApplications.Start(Seconds(1.0));
    Simulator::Schedule(Seconds(1.1), &CalculateThroughput);

    //    sinkApp.Start(Seconds(0.0));
    //    serverApp.Start(Seconds(1.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

     AnimationInterface anim("l7r1.xml");
    //    anim.SetMaxPktsPerTraceFile(1000000);

    Simulator::Stop(Seconds(simulationTime + 1));
    Simulator::Run();
    double averageThroughput = ((sink->GetTotalRx() * 8) / (1e6 * simulationTime));
    Simulator::Destroy();
    if (averageThroughput < 2.5)
    {
        std::cout << "Obtained throughput is not in the expected boundaries!"
                  << "\n";
        exit(1);
    }
    std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s" << std::endl;
    return 0;
}
