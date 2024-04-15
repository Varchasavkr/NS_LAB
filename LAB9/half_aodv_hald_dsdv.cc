// Define the number of nodes for each set
uint32_t nAodvNodes = 2;
uint32_t nDsdvNodes = 2;

NodeContainer p2pNodes;
p2pNodes.Create (6); // Total 6 nodes

// Split nodes for AODV and DSDV
NodeContainer aodvNodes = NodeContainer(p2pNodes.Get(0), p2pNodes.Get(1));
NodeContainer dsdvNodes = NodeContainer(p2pNodes.Get(2), p2pNodes.Get(3));

// Remaining nodes for CSMA
NodeContainer csmaNodes;
csmaNodes.Add(p2pNodes.Get(4));
csmaNodes.Add(p2pNodes.Get(5));

// Remaining code remains unchanged...

// Apply AODV routing protocol
AodvHelper aodv;
stack.SetRoutingHelper (aodv);
Ipv4InterfaceContainer aodvInterfaces = ipv4.Assign (aodvDevices);
Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

// Apply DSDV routing protocol
DsdvHelper dsdv;
stack.SetRoutingHelper (dsdv);
Ipv4InterfaceContainer dsdvInterfaces = ipv4.Assign (dsdvDevices);
Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

