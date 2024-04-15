#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"


using namespace ns3;

int main(int argc,char* argv[]) {
NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");
  bool tracing = true;
bool verbose = true;
uint32_t nCsma = 3;

CommandLine cmd;
cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

cmd.Parse (argc, argv);

if (verbose)
  {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }

nCsma = nCsma == 0 ? 1 : nCsma;


NodeContainer p2pNodes2;
p2pNodes2.Create (3);

NodeContainer csmaNodes;
csmaNodes.Add (p2pNodes2.Get (1));
csmaNodes.Add (p2pNodes2.Get (2));
csmaNodes.Create (nCsma);

PointToPointHelper pointToPoint1;
pointToPoint1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
pointToPoint1.SetChannelAttribute ("Delay", StringValue ("2ms"));

PointToPointHelper pointToPoint2;
pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
pointToPoint2.SetChannelAttribute ("Delay", StringValue ("2ms"));

NetDeviceContainer p2pDevices1;
p2pDevices1 = pointToPoint1.Install (p2pNodes2.Get(0),p2pNodes2.Get(1));

NetDeviceContainer p2pDevices2;
p2pDevices2 = pointToPoint2.Install (p2pNodes2.Get(1),p2pNodes2.Get(2));

CsmaHelper csma;
csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

NetDeviceContainer csmaDevices;
csmaDevices = csma.Install (csmaNodes);

InternetStackHelper stack;

stack.Install (p2pNodes2.Get (0));

stack.Install (csmaNodes);

Ipv4AddressHelper address;
address.SetBase ("10.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer p2pInterfaces1;
p2pInterfaces1 = address.Assign (p2pDevices1);


address.SetBase ("10.1.2.0", "255.255.255.0");
Ipv4InterfaceContainer p2pInterfaces2;
p2pInterfaces2 = address.Assign (p2pDevices2);


address.SetBase ("10.1.3.0", "255.255.255.0");
Ipv4InterfaceContainer csmaInterfaces;
csmaInterfaces = address.Assign (csmaDevices);

UdpEchoServerHelper echoServer (9);

ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (nCsma+1));
serverApps.Start (Seconds (1.0));
serverApps.Stop (Seconds (10.0));


UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma+1), 9);
echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

ApplicationContainer clientApps1 = echoClient.Install (p2pNodes2.Get (0));

clientApps1.Stop (Seconds (10.0));


Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


pointToPoint1.EnablePcapAll ("second");
pointToPoint2.EnablePcapAll ("second");
csma.EnablePcap ("second", csmaDevices.Get (1), true);

MobilityHelper mobility;

mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
            "MinX", DoubleValue (0.0), "MinY", DoubleValue (0.0),"DeltaX", DoubleValue (5.0), "DeltaY", DoubleValue (10.0),
             "GridWidth", UintegerValue (5), "LayoutType", StringValue ("RowFirst"));

mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

mobility.Install (p2pNodes2);

mobility.Install (csmaNodes);


AnimationInterface anim ("sample.xml");

 if (tracing)
    {
      AsciiTraceHelper ascii;
      pointToPoint1.EnableAsciiAll (ascii.CreateFileStream ("sample.tr"));
      pointToPoint1.EnablePcapAll ("sample", false);
  
    }

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

