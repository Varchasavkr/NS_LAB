#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/aodv-module.h" // Include AODV module
#include "ns3/dsdv-module.h" // Include DSDV module
#include "ns3/core-module.h"
using namespace ns3;


NS_LOG_COMPONENT_DEFINE("MyComponent");

int main()
{
  // Define the number of nodes for each set
  
  // Create nodes
  LogComponentEnable("MyComponent", LOG_LEVEL_INFO); // Set log level for your component

    NS_LOG_INFO("Hello, NS-3!"); // Log 
  NS_LOG_INFO ("asdfasdf.");
  NodeContainer p2pNodes;
  p2pNodes.Create(6); // Total 6 nodes

  // Split nodes for AODV and DSDV
  NodeContainer aodvNodes = NodeContainer(p2pNodes.Get(0), p2pNodes.Get(1));
  NodeContainer dsdvNodes = NodeContainer(p2pNodes.Get(2), p2pNodes.Get(3));

  // Remaining nodes for CSMA
  NodeContainer csmaNodes;
  csmaNodes.Add(p2pNodes.Get(4));
  csmaNodes.Add(p2pNodes.Get(5));

  // Point-to-Point attributes
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  // Install point-to-point devices
  NetDeviceContainer d0d1 = pointToPoint.Install(NodeContainer(p2pNodes.Get(0), p2pNodes.Get(1)));
  NetDeviceContainer d1d2 = pointToPoint.Install(NodeContainer(p2pNodes.Get(2), p2pNodes.Get(3)));

  // CSMA attributes
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

  // Install CSMA devices
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);

  // Internet stack
  InternetStackHelper stack;
  stack.Install(aodvNodes);
  stack.Install(dsdvNodes);
  stack.Install(csmaNodes);

  // Assign IP Addresses
  Ipv4AddressHelper ipv4;

  // Assign IP addresses for AODV nodes
  ipv4.SetBase("172.16.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = ipv4.Assign(d0d1);

  // Assign IP addresses for DSDV nodes
  ipv4.SetBase("172.16.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

  // Assign IP addresses for CSMA nodes
  ipv4.SetBase("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces = ipv4.Assign(csmaDevices);

  // Apply AODV routing protocol to AODV nodes
  AodvHelper aodv;
  stack.SetRoutingHelper(aodv);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Apply DSDV routing protocol to DSDV nodes
  DsdvHelper dsdv;
  stack.SetRoutingHelper(dsdv);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Set up applications
  UdpEchoServerHelper echoServer(9);
  ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(1));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  UdpEchoClientHelper echoClient(i1i2.GetAddress(1), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(6));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps = echoClient.Install(p2pNodes.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  // Create a non-participating node
  p2pNodes.Create(1);
	
  // Set up animation
  


  // Run simulation
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

