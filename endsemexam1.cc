/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/animation-interface.h"
#include "ns3/energy-module.h"
#include "ns3/wifi-radio-energy-model-helper.h"
#include "ns3/constant-velocity-mobility-model.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("SecondScriptExample");

void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
  nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
}

void Stop_node(Ptr<ConstantVelocityMobilityModel> m)
{
  m->SetVelocity(Vector(0.0,0.0,0.0));
}

void Join_link (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
  nodeA->GetObject<Ipv4> ()->SetUp(interfaceA);
  nodeB->GetObject<Ipv4> ()->SetUp (interfaceB);
}

int 
main (int argc, char *argv[])
{

  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);

  NodeContainer nodes,csma_nodes,p2p1,p2p2,p2p3,wireless_sta,wireless_ap, wireless;
  nodes.Create (11);
  
  //csma N9-N11
  csma_nodes.Add(nodes.Get(8));//N9
  csma_nodes.Add(nodes.Get(9));//N10
  csma_nodes.Add(nodes.Get(10));//N11
  
  //Droptail
  p2p1.Add(nodes.Get(9));//N10
  p2p1.Add(nodes.Get(7));//N8

  //9mbps,5ms
  p2p2.Add(nodes.Get(6));//N7
  p2p2.Add(nodes.Get(7));//N8
  
  //9mbps,5ms
  p2p3.Add(nodes.Get(4));//N5
  p2p3.Add(nodes.Get(6));//N7
  
  //wireless
  wireless_ap.Add(nodes.Get(4));//N5
  for(int i = 0; i < 6; i++)
  {
    if(i == 4)
      continue;
    wireless_sta.Add(nodes.Get(i));//N1-N6
  }
   //wifi
  WifiMacHelper wifiMac;
  WifiHelper wifiHelper;
  wifiHelper.SetStandard (ns3::WIFI_STANDARD_80211n_2_4GHZ);

  /* Set up Legacy Channel */
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue (5e9));

  /* Setup Physical Layer */
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode", StringValue ("HtMcs7"),
                                      "ControlMode", StringValue ("HtMcs0"));

  
  PointToPointHelper pointToPoint;
  NetDeviceContainer p2pd1,p2pd2,p2pd3,csmad;
  
  //9mbps,5ms
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("5ms"));
  p2pd1 = pointToPoint.Install (p2p2);
  p2pd2 = pointToPoint.Install (p2p3);
  
  //droptail
  pointToPoint.SetQueue ("ns3::DropTailQueue", "MaxSize",StringValue ("50p"));
  p2pd3 = pointToPoint.Install (p2p1);
  
  //csma
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("10ms"));
  csmad = csma.Install (csma_nodes);
  
  /* Configure AP */
  Ssid ssid = Ssid ("network");
  wifiMac.SetType ("ns3::ApWifiMac",
                   "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevice;
  apDevice = wifiHelper.Install (wifiPhy, wifiMac, wireless_ap);

  /* Configure STA */
  wifiMac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid));

  NetDeviceContainer staDevices, devices;
  staDevices = wifiHelper.Install (wifiPhy, wifiMac, wireless_sta);
  
  
  
  /* Mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (20, 20, 0.0));
  positionAlloc->Add (Vector (40.0, 10.0, 0.0));
  positionAlloc->Add (Vector (20, 70, 0.0));
  positionAlloc->Add (Vector (50, 120, 0.0));
  positionAlloc->Add (Vector (100, 10, 0.0));
  positionAlloc->Add (Vector (70, 70, 0.0));

  positionAlloc->Add (Vector (100, 100, 0.0));
  positionAlloc->Add (Vector (120, 120, 0.0));
  positionAlloc->Add (Vector (130, 100, 0.0));
  positionAlloc->Add (Vector (140, 130, 0.0));
  positionAlloc->Add (Vector (150, 100, 0.0));

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (nodes);
  cout << 1 << '\n';
  //movility with velocity
  
  Ptr<ConstantVelocityMobilityModel> m2 = DynamicCast<ConstantVelocityMobilityModel>(nodes.Get(1)->GetObject<MobilityModel> ());
  Ptr<ConstantVelocityMobilityModel> m3 = DynamicCast<ConstantVelocityMobilityModel>(nodes.Get(2)->GetObject<MobilityModel> ());
  Ptr<ConstantVelocityMobilityModel> m6 = DynamicCast<ConstantVelocityMobilityModel>(nodes.Get(5)->GetObject<MobilityModel> ());
  m2->SetVelocity(Vector(5,0,0));
  m3->SetVelocity(Vector(5,0,0));
  m6->SetVelocity(Vector(5,0,0));
  
  Simulator::Schedule (Seconds (10), &Stop_node, m2);
  Simulator::Schedule (Seconds (10), &Stop_node, m3);
  Simulator::Schedule (Seconds (10), &Stop_node, m6);
  
  InternetStackHelper stack;
  stack.Install (nodes);
  
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pi1,p2pi2,p2pi3,csmai;
  
  //6mbps,10ms
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pi1 = address.Assign (p2pd1);
  
  //9mbps,15ms
  address.SetBase ("10.1.2.0", "255.255.255.0");
  p2pi2 = address.Assign (p2pd2);
  
  //droptail
  address.SetBase ("10.1.3.0", "255.255.255.0");
  p2pi3 = address.Assign (p2pd3);
  
  //csma
  address.SetBase ("10.1.4.0", "255.255.255.0");
  csmai = address.Assign (csmad);
  
  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface;
  apInterface = address.Assign (apDevice);
  Ipv4InterfaceContainer staInterface;
  staInterface = address.Assign (staDevices);


  Ipv4GlobalRoutingHelper routingHelper;
  routingHelper.PopulateRoutingTables();
  
  uint16_t port = 9;   // Discard port (RFC 863)
  OnOffHelper TcpOnoff ("ns3::TcpSocketFactory", Address (InetSocketAddress (staInterface.GetAddress(0), port)));
  TcpOnoff.SetConstantRate (DataRate ("448kb/s"));
  ApplicationContainer TcpApps = TcpOnoff.Install (nodes.Get (8));//N9
  TcpApps.Start (Seconds (1.0));
  TcpApps.Stop (Seconds (5.0));

  // Create a packet sink to receive these packets
  PacketSinkHelper TcpSink ("ns3::TcpSocketFactory", Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  TcpApps = TcpSink.Install (nodes.Get (0));//N1
  TcpApps.Start (Seconds (0.5));
  TcpApps.Stop (Seconds (6.0));
  
  

  /* Install TCP Receiver on the access point */
  PacketSinkHelper WsinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer WsinkApp = WsinkHelper.Install (nodes.Get(4));//N5

  /* Install TCP/UDP Transmitter on the station */
  OnOffHelper Wserver ("ns3::TcpSocketFactory", (InetSocketAddress (p2pi3.GetAddress (0), 9)));
  Wserver.SetAttribute ("PacketSize", UintegerValue (1024));
  Wserver.SetAttribute ("MaxBytes", UintegerValue(90*1024));
  Wserver.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  Wserver.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  Wserver.SetAttribute ("DataRate", StringValue("10240bps"));
  ApplicationContainer WserverApp = Wserver.Install (nodes.Get(10));//N11

  /* Start Applications */
  WsinkApp.Start (Seconds (5.0));
  WsinkApp.Stop (Seconds(16.0));
  WserverApp.Start (Seconds (6.0));
  WserverApp.Stop (Seconds(15.0));
  
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
  routingHelper.PrintRoutingTableAt (Seconds (7.0), nodes.Get(6), routingStream);
  routingHelper.PrintRoutingTableAt (Seconds (8.5), nodes.Get(6), routingStream);
  routingHelper.PrintRoutingTableAt (Seconds (10.5), nodes.Get(6), routingStream);
  
  Simulator::Schedule (Seconds (8.0), &TearDownLink, nodes.Get(7), nodes.Get(9), 1, 0);
  Simulator::Schedule(Seconds(8.2), &(Ipv4GlobalRoutingHelper::RecomputeRoutingTables));
  Simulator::Schedule (Seconds (10.1), &Join_link, nodes.Get(7), nodes.Get(9), 1, 0);
  Simulator::Schedule(Seconds(10.3), &(Ipv4GlobalRoutingHelper::RecomputeRoutingTables));
  cout << 1 << '\n';
  AnimationInterface Anim("pract.xml");
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("mixed-global-routing.tr");
  pointToPoint.EnableAsciiAll (stream);
  csma.EnableAsciiAll (stream);
  wifiPhy.EnableAsciiAll (stream);
  
  Simulator::Stop (Seconds(17.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
