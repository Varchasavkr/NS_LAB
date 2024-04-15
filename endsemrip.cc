#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-routing-table-entry.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("exam");

 void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
 {
   nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
   nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
 }

int main (int argc, char *argv[])
{
  bool verbose = true;
  bool showPings = false;
  uint32_t nCsma1 = 1;
  uint32_t nCsma2 = 1;
  uint32_t maxBytes = 0;
  std::string SplitHorizon ("PoisonReverse");
  bool printRoutingTables = true;
  
  std :: string animFile = "exam336.xml";
//"scratch/myfirst --ns3::PointToPointNetDevice::DataRate=5Mbps"
  CommandLine cmd;
//cmd.AddValue ("verbose", "turn on log components", verbose);
//cmd.AddValue ("printRoutingTables", "Print routing tables at 30, 60 and 90 seconds",
//printRoutingTables);
//cmd.AddValue ("showPings", "Show Ping6 reception", showPings);
//cmd.AddValue ("splitHorizonStrategy", "Split Horizon strategy to use (NoSplitHorizon,
//SplitHorizon, PoisonReverse)", SplitHorizon);
cmd.Parse (argc, argv);

  
  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

 // nCsma = nCsma == 0 ? 1 : nCsma;

  NodeContainer p2pNodes;
  p2pNodes.Create (9);

  NodeContainer csmaNodes1;
  csmaNodes1.Add (p2pNodes.Get (2));
  csmaNodes1.Create (nCsma1);
csmaNodes1.Add (p2pNodes.Get (4));

NodeContainer csmaNodes2;
  csmaNodes2.Add (p2pNodes.Get (8));
  csmaNodes2.Create (nCsma2);

  PointToPointHelper pointToPoint; //0-1
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("3Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("5ms"));
  pointToPoint.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));

  PointToPointHelper pointToPoint2;//1-2
  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("6Mbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("10ms"));
  pointToPoint2.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));

PointToPointHelper pointToPoint3;//2-3
  pointToPoint3.SetDeviceAttribute ("DataRate", StringValue ("3Mbps"));
  pointToPoint3.SetChannelAttribute ("Delay", StringValue ("5ms"));
pointToPoint3.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));

PointToPointHelper pointToPoint4;//4-5
  pointToPoint4.SetDeviceAttribute ("DataRate", StringValue ("9Mbps"));
  pointToPoint4.SetChannelAttribute ("Delay", StringValue ("15ms"));
pointToPoint4.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));

PointToPointHelper pointToPoint5;//5-6
  pointToPoint5.SetDeviceAttribute ("DataRate", StringValue ("9Mbps"));
  pointToPoint5.SetChannelAttribute ("Delay", StringValue ("15ms"));
pointToPoint5.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));

PointToPointHelper pointToPoint6;//5-7
  pointToPoint5.SetDeviceAttribute ("DataRate", StringValue ("9Mbps"));
  pointToPoint5.SetChannelAttribute ("Delay", StringValue ("15ms"));
pointToPoint6.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));

PointToPointHelper pointToPoint7;//7-8
  pointToPoint5.SetDeviceAttribute ("DataRate", StringValue ("9Mbps"));
  pointToPoint5.SetChannelAttribute ("Delay", StringValue ("15ms"));
pointToPoint7.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));

PointToPointHelper pointToPoint8;//6-8
  pointToPoint5.SetDeviceAttribute ("DataRate", StringValue ("9Mbps"));
  pointToPoint5.SetChannelAttribute ("Delay", StringValue ("15ms"));
pointToPoint8.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));



  NetDeviceContainer p2pDevice1, p2pDevice2, p2pDevice3,p2pDevice4,p2pDevice5,p2pDevice6,p2pDevice7,p2pDevice8;
  p2pDevice1 = pointToPoint.Install (p2pNodes.Get(0), p2pNodes.Get(1));
  p2pDevice2 = pointToPoint2.Install (p2pNodes.Get(1), p2pNodes.Get(2));
p2pDevice3 = pointToPoint3.Install (p2pNodes.Get(2), p2pNodes.Get(3));
p2pDevice4 = pointToPoint4.Install (p2pNodes.Get(4), p2pNodes.Get(5));
p2pDevice5 = pointToPoint5.Install (p2pNodes.Get(5), p2pNodes.Get(6));
p2pDevice6 = pointToPoint6.Install (p2pNodes.Get(5), p2pNodes.Get(7));
p2pDevice7 = pointToPoint7.Install (p2pNodes.Get(7), p2pNodes.Get(8));
p2pDevice8 = pointToPoint8.Install (p2pNodes.Get(8), p2pNodes.Get(6));


  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));

  
  

  NetDeviceContainer csmaDevices1, csmaDevices2;
  csmaDevices1 = csma.Install (csmaNodes1);
CsmaHelper csma2;
  csma2.SetChannelAttribute ("DataRate", StringValue ("70Mbps"));
  csma2.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (13)));

csmaDevices2 = csma.Install (csmaNodes2);

  InternetStackHelper stack;
  //stack.Install (p2pNodes);
 stack.Install (p2pNodes.Get (0));
 stack.Install (p2pNodes.Get (1));
 stack.Install (p2pNodes.Get (7));
 stack.Install (p2pNodes.Get (5));
 stack.Install (p2pNodes.Get (6));
 stack.Install (p2pNodes.Get (3));
  //stack.Install (csmaNodes);
  stack.Install (csmaNodes1);
  stack.Install (csmaNodes2);
  
 
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterface1, p2pInterface2, p2pInterface3,p2pInterface4,p2pInterface5,p2pInterface6,p2pInterface7,p2pInterface8;
  p2pInterface1 = address.Assign (p2pDevice1);
  p2pInterface2 = address.Assign (p2pDevice2);
  p2pInterface3 = address.Assign (p2pDevice3);
  p2pInterface4 = address.Assign (p2pDevice4);
  p2pInterface5 = address.Assign (p2pDevice5);
  p2pInterface6 = address.Assign (p2pDevice6);
  p2pInterface7 = address.Assign (p2pDevice7);
  p2pInterface8 = address.Assign (p2pDevice8);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces1;
  csmaInterfaces1 = address.Assign (csmaDevices1);
address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces2;
  csmaInterfaces2 = address.Assign (csmaDevices2);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (p2pNodes.Get (0));
  serverApps.Start (Seconds (1.09));
  serverApps.Stop (Seconds (12.0));

  UdpEchoClientHelper echoClient (p2pInterface1.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (5500));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (csmaNodes2.Get(0));
  clientApps.Start (Seconds (1.09));
  clientApps.Stop (Seconds (12.0));
  
  
 BulkSendHelper source ("ns3::TcpSocketFactory",
InetSocketAddress (p2pInterface1.GetAddress (0), 9));
source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
ApplicationContainer sourceApps = source.Install (p2pNodes.Get (0));
sourceApps.Start (Seconds (5.0));
sourceApps.Stop (Seconds (10.0));
//
// Create a PacketSinkApplication and install it on N10
//
PacketSinkHelper sink ("ns3::TcpSocketFactory",
InetSocketAddress (Ipv4Address::GetAny (), 9));
ApplicationContainer sinkApps = sink.Install (p2pNodes.Get (8));
sinkApps.Start (Seconds (5.0));
sinkApps.Stop (Seconds (10.0));
//

AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("exam336.tr"));
  pointToPoint2.EnableAsciiAll(ascii.CreateFileStream("exam336_3.tr"));
  //csma.EnableAsciiAll(ascii.CreateFileStream("exam336.tr"));
  pointToPoint8.EnableAsciiAll(ascii.CreateFileStream("exam336_2.tr"));
  
  AnimationInterface anim ("exam336.xml");
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
