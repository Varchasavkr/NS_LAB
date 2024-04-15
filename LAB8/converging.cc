
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/packet-sink.h"
#include "ns3/netanim-module.h"
using namespace ns3;
NS_LOG_COMPONENT_DEFINE("ThirdScriptExample");
int main(int argc, char *argv[])
{
    bool verbose = true;
    uint32_t nWifi = 5;
    bool tracing = false;
    CommandLine cmd(__FILE__);
    cmd.AddValue("nWifi", "Number of wifi STA devices", nWifi);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue("tracing", "Enable pcap tracing", tracing);
    cmd.Parse(argc, argv);
    if (nWifi > 18)
    {
        std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
        return 1;
    }
    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
        LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
        LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    }
    NodeContainer p2pNodes;
    p2pNodes.Create(2);
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create(nWifi);
    NodeContainer wifiApNode = p2pNodes.Get(0);
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());
    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");
    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false));
    NetDeviceContainer staDevices;
    staDevices = wifi.Install(phy, mac, wifiStaNodes);
    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid));
    NetDeviceContainer apDevices;
    apDevices = wifi.Install(phy, mac, wifiApNode);
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(0.0),
                                  "MinY", DoubleValue(0.0),
                                  "DeltaX", DoubleValue(5.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(3),
                                  "LayoutType", StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
    mobility.Install(wifiStaNodes);
    Ptr<ConstantVelocityMobilityModel> cvmm1 = wifiStaNodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
    cvmm1->SetPosition(Vector(500, 0, 0));
    cvmm1->SetVelocity(Vector(-50, 0, 0));
    Ptr<ConstantVelocityMobilityModel> cvmm2 = wifiStaNodes.Get(1)->GetObject<ConstantVelocityMobilityModel>();
    cvmm2->SetPosition(Vector(-500, 0, 0));
    cvmm2->SetVelocity(Vector(50, 0, 0));
    Ptr<ConstantVelocityMobilityModel> cvmm3 = wifiStaNodes.Get(2)->GetObject<ConstantVelocityMobilityModel>();
    cvmm3->SetPosition(Vector(0, 500, 0));
    cvmm3->SetVelocity(Vector(0, -50, 0));
    Ptr<ConstantVelocityMobilityModel> cvmm4 = wifiStaNodes.Get(3)->GetObject<ConstantVelocityMobilityModel>();
    cvmm4->SetPosition(Vector(0, -500, 0));
    cvmm4->SetVelocity(Vector(0, 50, 0));
    Ptr<ConstantVelocityMobilityModel> cvmm5 = wifiStaNodes.Get(4)->GetObject<ConstantVelocityMobilityModel>();
    cvmm5->SetVelocity(Vector(25, 10, 0));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);
    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(1));
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);
    address.SetBase("10.1.2.0", "255.255.255.0");
    address.Assign(staDevices);
    address.Assign(apDevices);
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(p2pNodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));
    uint16_t port2 = 18;
    OnOffHelper source("ns3::TcpSocketFactory", InetSocketAddress(p2pInterfaces.GetAddress(1), port2));
    source.SetAttribute("DataRate", StringValue("200Kbps"));
    ApplicationContainer senders = source.Install(wifiStaNodes.Get(nWifi - 1));
    senders.Start(Seconds(2.0));
    senders.Stop(Seconds(10.0));
    PacketSinkHelper sink(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port2));
    ApplicationContainer sinkApps = sink.Install(p2pNodes.Get(1));
    sinkApps.Start(Seconds(0.0));
    sinkApps.Stop(Seconds(10.0));
    UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(20));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(3.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
    ApplicationContainer clientApps =
        echoClient.Install(wifiStaNodes.Get(nWifi - 1));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    AnimationInterface anim("wifi_prac.xml");
    anim.SetConstantPosition(wifiStaNodes.Get(0), 0.0, 0.0);
    anim.SetConstantPosition(wifiStaNodes.Get(1), 100.0, 100.0);
    anim.SetConstantPosition(wifiStaNodes.Get(2), 200.0, 200.0);
    anim.SetConstantPosition(wifiStaNodes.Get(3), 0.0, 200.0);
    anim.SetConstantPosition(wifiStaNodes.Get(4), 100.0, 0.0);
    Simulator::Stop(Seconds(10.0));
    if (tracing == true)
    {
        AsciiTraceHelper ascii;
        pointToPoint.EnableAscii("source", p2pDevices.Get(1));
        phy.EnableAscii("third", apDevices.Get(0));
        pointToPoint.EnableAscii("third", p2pDevices.Get(0));
    }
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
