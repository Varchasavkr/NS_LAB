#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("lab1q1");

int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);
  
    Time::SetResolution(Time::NS);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    NodeContainer nodes;
    nodes.Create(4);

    // Link between n0 and n2
    PointToPointHelper pointToPoint1;
    pointToPoint1.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint1.SetChannelAttribute("Delay", StringValue("2ms"));
    pointToPoint1.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("3p"));
    NetDeviceContainer device1 = pointToPoint1.Install(nodes.Get(0), nodes.Get(1));

    // Link between n1 and n2
    PointToPointHelper pointToPoint2;
    pointToPoint2.SetDeviceAttribute("DataRate", StringValue("8Mbps"));
    pointToPoint2.SetChannelAttribute("Delay", StringValue("4ms"));
    pointToPoint2.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("3p"));
    NetDeviceContainer device2 = pointToPoint2.Install(nodes.Get(1), nodes.Get(2));

    // Link between n2 and n3
    PointToPointHelper pointToPoint3;
    pointToPoint3.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint3.SetChannelAttribute("Delay", StringValue("4ms"));
    pointToPoint3.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("3p"));
    NetDeviceContainer device3 = pointToPoint3.Install(nodes.Get(2), nodes.Get(3));

    InternetStackHelper stack;
    stack.Install(nodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer if1 = address.Assign(device1);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer if2 = address.Assign(device2);

    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer if3 = address.Assign(device3);
  
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(3)); // Install server on n3
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(if3.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(3));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));
  
    AnimationInterface anim("l1q2.xml");
    anim.SetConstantPosition(nodes.Get(0), 0.0, 10.0);
    anim.SetConstantPosition(nodes.Get(1), 10.0, 20.0);
    anim.SetConstantPosition(nodes.Get(2), 20.0, 30.0);
    anim.SetConstantPosition(nodes.Get(3), 30.0, 40.0); // Adjusted position of n3
  
    Simulator::Run();
    Simulator::Destroy();
  
    return 0;
}

