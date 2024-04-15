
 
#include "ns3/core-module.h"

#include "ns3/network-module.h"

#include "ns3/csma-module.h"

#include "ns3/internet-module.h"

#include "ns3/point-to-point-module.h"

#include "ns3/applications-module.h"

#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/netanim-module.h"

#include "ns3/aodv-module.h" // Include AODV module
 
using namespace ns3;
 
NS_LOG_COMPONENT_DEFINE ("aodv_wired");
 
int main()

{

  uint32_t nCsma = 3;

  NodeContainer p2pNodes;

  p2pNodes.Create (3);

  NodeContainer n0n1 = NodeContainer (p2pNodes.Get (0), p2pNodes.Get(1));

  NodeContainer n1n2 = NodeContainer (p2pNodes.Get (1), p2pNodes.Get(2));

  NodeContainer csmaNodes;

  csmaNodes.Add (p2pNodes.Get (1));

  csmaNodes.Add (p2pNodes.Get (2));

  csmaNodes.Create (nCsma);
 
  PointToPointHelper pointToPoint;

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));

  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer d0d1 = pointToPoint.Install (n0n1);

  NetDeviceContainer d1d2 = pointToPoint.Install (n1n2);

  CsmaHelper csma;

  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));

  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
 
  NetDeviceContainer csmaDevices;

  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;

  stack.Install (p2pNodes.Get(0));

  stack.Install (csmaNodes);
 
  NS_LOG_INFO ("Assign IP Addresses.");

  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("172.16.1.0", "255.255.255.0");

  Ipv4InterfaceContainer i0i1 = ipv4.Assign (d0d1);
 
  ipv4.SetBase ("172.16.2.0", "255.255.255.0");

  Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);
 
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");

  Ipv4InterfaceContainer csmaInterfaces;

  csmaInterfaces = ipv4.Assign (csmaDevices);
 
  // Apply AODV routing protocol

  AodvHelper aodv;

  stack.SetRoutingHelper (aodv);
 
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
 
  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get(nCsma));

  serverApps.Start (Seconds (1.0));

  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (i1i2.GetAddress (1), 9);

  echoClient.SetAttribute ("MaxPackets", UintegerValue (6));

  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));

  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
 
  ApplicationContainer clientApps = echoClient.Install (p2pNodes.Get (0));

  clientApps.Start (Seconds (2.0));

  clientApps.Stop (Seconds (10.0));

  p2pNodes.Create (1); //creates the 7 th node that does not participate

  AnimationInterface anim ("l9q1mod.xml");

  anim.SetConstantPosition (p2pNodes.Get(0), 1.0, 42.0);

  anim.SetConstantPosition (p2pNodes.Get(1), 9.0, 42.0);

  anim.SetConstantPosition (p2pNodes.Get(2), 19.0, 42.0);

  anim.SetConstantPosition (csmaNodes.Get(2), 30.0, 42.0);

  anim.SetConstantPosition (csmaNodes.Get(3), 40.0, 30.0);

  anim.SetConstantPosition (csmaNodes.Get(4), 45.0, 20.0);

  anim.SetConstantPosition (p2pNodes.Get(3), 50.0, 10.0);

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
 
