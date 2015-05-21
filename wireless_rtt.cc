/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* Skeleton code wireless-tcp-bulk-send.cc
 */

#include <string>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/athstats-helper.h"
#include "ns3/netanim-module.h"


using namespace ns3;
using namespace std;


NS_LOG_COMPONENT_DEFINE ("WirelessRtt");

bool firstCwnd = true;
bool firstSshThr = true;
bool firstRtt = true;
bool firstRto = true;
bool firstRttVar = true;
bool firstRealRtt = true;
bool firstDelta = true;


Ptr<OutputStreamWrapper> cWndStream;
Ptr<OutputStreamWrapper> ssThreshStream;
Ptr<OutputStreamWrapper> rttStream;
Ptr<OutputStreamWrapper> rtoStream;

Ptr<OutputStreamWrapper> rttvarStream;
Ptr<OutputStreamWrapper> rrttStream;
Ptr<OutputStreamWrapper> deltaStream;

uint32_t cWndValue;
uint32_t ssThreshValue;



/*****************************************************************************/
/*CALLBACK FUNCTIONS FOR TRACES                                               */
/*****************************************************************************/
static void
CwndTracer (uint32_t oldval, uint32_t newval)
{
  if (firstCwnd)
    {
      *cWndStream->GetStream () << "0.0 " << oldval << std::endl;
      firstCwnd = false;
    }
  *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  cWndValue = newval;

  if (!firstSshThr)
    {
      *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << ssThreshValue << std::endl;
    }
}

static void
SsThreshTracer (uint32_t oldval, uint32_t newval)
{
  if (firstSshThr)
    {
      *ssThreshStream->GetStream () << "0.0 " << oldval << std::endl;
      firstSshThr = false;
    }
  *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  ssThreshValue = newval;

  if (!firstCwnd)
    {
      *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << cWndValue << std::endl;
    }
}

static void
RttTracer (Time oldval, Time newval)
{
  if (firstRtt)
    {
      *rttStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRtt = false;
    }
  *rttStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}

static void
RtoTracer (Time oldval, Time newval)
{
  if (firstRto)
    {
      *rtoStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRto = false;
    }
  *rtoStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}
static void
RttVarTracer (Time oldval, Time newval)
{
  if (firstRttVar)
    {
      *rttvarStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRttVar = false;
    }
  *rttvarStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}




static void
RealRttTracer (Time oldval, Time newval)
{
  if (firstRealRtt)
    {
      *rrttStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRealRtt = false;
    }
  *rrttStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}


static void
DeltaTracer (Time oldval, Time newval)
{
  if (firstDelta)
    {
      *deltaStream->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstDelta = false;
    }
  *deltaStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}



/*****************************************************************************/

/*****************************************************************************/
/*SETTING UP FILES FOR STORING TRACE VALUES                                  */
/*****************************************************************************/
static void
TraceCwnd (std::string cwnd_tr_file_name)
{
  AsciiTraceHelper ascii;
  cWndStream = ascii.CreateFileStream (cwnd_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/CongestionWindow", MakeCallback (&CwndTracer));
}

static void
TraceSsThresh (std::string ssthresh_tr_file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream = ascii.CreateFileStream (ssthresh_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/SlowStartThreshold", MakeCallback (&SsThreshTracer));
}

static void
TraceRtt (std::string rtt_tr_file_name)
{
  AsciiTraceHelper ascii;
  rttStream = ascii.CreateFileStream (rtt_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/RTT", MakeCallback (&RttTracer));
}

static void
TraceRto (std::string rto_tr_file_name)
{
  AsciiTraceHelper ascii;
  rtoStream = ascii.CreateFileStream (rto_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/RTO", MakeCallback (&RtoTracer));
}
static void
TraceRealRtt (std::string rrtt_tr_file_name)
{
  AsciiTraceHelper ascii;
  rrttStream = ascii.CreateFileStream (rrtt_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/realRTT", MakeCallback (&RealRttTracer));
}
static void
TraceRttVar (std::string rttvar_tr_file_name)
{
  AsciiTraceHelper ascii;
  rttvarStream = ascii.CreateFileStream (rttvar_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/RTTvar", MakeCallback (&RttVarTracer));
}
static void
TraceDelta (std::string delta_tr_file_name)
{
  AsciiTraceHelper ascii;
  deltaStream = ascii.CreateFileStream (delta_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*/Delta", MakeCallback (&DeltaTracer));
}


//void get_throughput(Ptr<PacketSink> sink_a)
//{
//
//  // Throughput calculation.
//  // Transfer Size = sink->GetTotalRx()
//  // Throughput = Transfer Size/Time
//  // fprintf for writing simultaneuosly into file used for generating graphs
//
//  Time t = Simulator::Now();
//  printf("Current time:%f seconds.\n",t.GetSeconds());
//
//  //connection 1
//  int bytesTotal = sink_a->GetTotalRx();
//  float throughput = (bytesTotal*8.0)/1000/t.GetSeconds();
//  printf("Throughput at sink : %f Kbps.\n",throughput);
//  //printf("%f\t",t.GetSeconds());
//  //printf("%f\n",throughput);
//
//  if(t.GetSeconds()<50.1)
//  {
//    Simulator::Schedule(Seconds(0.5), &get_throughput, sink_a);
//  }
//}

void
PhyTxTrace (std::string context, Ptr<const Packet> packet, WifiMode mode, WifiPreamble preamble, uint8_t txPower)
{
      std::cout << "PHYTX mode=" << mode << " " << *packet << std::endl;
}


int
main (int argc, char *argv[])
{
  uint32_t maxBytes = 3000;
  double SimTime = 30.0;


  LogComponentEnable("WirelessRtt", LOG_LEVEL_INFO);

//  LogComponentEnable("ConstantRateWifiManager", LOG_LEVEL_INFO);
//  LogComponentEnable ("BulkSendApplication", LOG_LEVEL_INFO);
//  LogComponentEnable ("TcpSocketBase", LOG_LEVEL_LOGIC);

  std::string traceFile = "wireless.ns_movements";
  std::string tr_file_name = "";
  std::string cwnd_tr_file_name = "wireless.cwnd";
  std::string ssthresh_tr_file_name = "wireless.ssth";
  std::string rtt_tr_file_name = "wireless.rtt";
  std::string rto_tr_file_name = "wireless.rto";
  std::string rwin_tr_file_name = "wireless.rwin";
  std::string rttvar_tr_file_name = "";
  std::string rrtt_tr_file_name = "wireless.rrtt";
  std::string delta_tr_file_name = "wireless.delta";


  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
  Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
  Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
  Config::SetDefault ("ns3::TcpSocketBase::WindowScaling", BooleanValue (true));

//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer stas, ap;
  ap.Create(1);
  stas.Create(1);

  NS_LOG_INFO ("Create channels.");

//
// Explicitly create the point-to-point link required by the topology (shown above).

// Wifi Channel
  YansWifiChannelHelper wifiChannelHelper = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> wifiChannel = wifiChannelHelper.Create ();

// Physical Layer
  YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default ();
  wifiPhyHelper.SetChannel(wifiChannel);

// Mac Layer
  NqosWifiMacHelper wifiMacHelper = NqosWifiMacHelper::Default ();
  Ssid ssid = Ssid ("ns-3-ssid");

// Install the phy, mac on the station node
  wifiMacHelper.SetType ("ns3::StaWifiMac",
                        "Ssid", SsidValue (ssid),
                        "ActiveProbing", BooleanValue (false));
  WifiHelper wifiHelper = WifiHelper::Default ();
  wifiHelper.SetStandard(WIFI_PHY_STANDARD_80211a);
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("OfdmRate54Mbps"));
  //wifiHelper.SetRemoteStationManager("ns3::MinstrelWifiManager");

  NetDeviceContainer wifiContainer = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, stas);


// Install the phy, mac on the ap node
  wifiMacHelper.SetType ("ns3::ApWifiMac",
                       "Ssid", SsidValue (ssid));
  wifiHelper.SetStandard(WIFI_PHY_STANDARD_80211a);
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode", StringValue ("OfdmRate54Mbps"));
  //wifiHelper.SetRemoteStationManager("ns3::MinstrelWifiManager");

  NetDeviceContainer apContainer = wifiHelper.Install(wifiPhyHelper, wifiMacHelper, ap);


//
// Install the internet stack on the nodes
//
  InternetStackHelper internet;
  internet.Install (ap);
  internet.Install (stas);


//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (apContainer);
  Ipv4InterfaceContainer i1 = ipv4.Assign (wifiContainer);

/*Configure mobility -  DEBUGGING
//  Ns2MobilityHelper mobility(traceFile);
//  mobility.Install();
*/

// mobility.
MobilityHelper mobility;
Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

positionAlloc->Add (Vector (0.0, 0.0, 0.0));
positionAlloc->Add (Vector (1.0, 0.0, 15.0));
mobility.SetPositionAllocator (positionAlloc);

mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

mobility.Install (ap);
mobility.Install (stas);





  NS_LOG_INFO ("Create Applications.");

//
// Create a BulkSendApplication
//
  uint16_t port = 9;  // well-known echo port number


  BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (i1.GetAddress (0), port));
  //source.SetAttribute("SendSize", UintegerValue (1500));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (int(maxBytes* 100000)));


  ApplicationContainer sourceApps = source.Install (ap.Get (0));
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (SimTime));

//
// Create a PacketSinkApplication and install it on node 1

  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (stas.Get (0));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (SimTime));

  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
//
// Set up tracing if enabled
//
  if (true)
    {
      //AsciiTraceHelper ascii;
      //wifiHelper.EnableAsciiAll (ascii.CreateFileStream ("tcp-bulk-send.tr"));
      wifiPhyHelper.EnablePcapAll ("tcp-bulk-send", true);
    }
//
// Now, do the actual simulation.
//
  // Setting up Animation Interface nicely
//   AnimationInterface::SetNodeDescription (ap, "AP"); // Optional
//   AnimationInterface::SetNodeDescription (stas, "STA"); //b Optional
//   AnimationInterface::SetNodeColor (ap, 0, 255, 0); // Optional
//   AnimationInterface::SetNodeColor (stas, 255, 0, 0); // Optional

//  AnimationInterface anim ("BulkSend.xml");

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));

  //Config::Connect ("/NodeList/0/DeviceList/0/Phy/State/Tx", MakeCallback (&PhyTxTrace));

//  Time t = Simulator::Now();
//  std::cout << "Time: " << t.GetSeconds() << std::endl;
//  Simulator::Schedule(Seconds(0.1), &get_throughput, sink1);


if (tr_file_name.compare ("") != 0)
  {
    std::ofstream ascii;
    Ptr<OutputStreamWrapper> ascii_wrap;
    ascii.open (tr_file_name.c_str ());
    ascii_wrap = new OutputStreamWrapper (tr_file_name.c_str (), std::ios::out);
    internet.EnableAsciiIpv4All (ascii_wrap);
  }

if (cwnd_tr_file_name.compare ("") != 0)
  {
    Simulator::Schedule (Seconds (0.00001), &TraceCwnd, cwnd_tr_file_name);
  }

if (ssthresh_tr_file_name.compare ("") != 0)
  {
    Simulator::Schedule (Seconds (0.00001), &TraceSsThresh, ssthresh_tr_file_name);
  }
/*****estimate RTT**************************************************************/
if (rtt_tr_file_name.compare ("") != 0)
  {
    Simulator::Schedule (Seconds (0.00001), &TraceRtt, rtt_tr_file_name);
  }

if (rto_tr_file_name.compare ("") != 0)
  {
    Simulator::Schedule (Seconds (0.00001), &TraceRto, rto_tr_file_name);
  }

/*****RTT VARIANCE**************************************************************/
if (rttvar_tr_file_name.compare ("") != 0)
  {
    Simulator::Schedule (Seconds (0.00001), &TraceRttVar, rttvar_tr_file_name);
  }
/*****REAL RTT**************************************************************/
if (rrtt_tr_file_name.compare ("") != 0)
  {
    Simulator::Schedule (Seconds (0.00001), &TraceRealRtt, rrtt_tr_file_name);
  }

  /*****Delta**************************************************************/
if (delta_tr_file_name.compare ("") != 0)
  {
    Simulator::Schedule (Seconds (0.00001), &TraceDelta, delta_tr_file_name);
  }


  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (SimTime));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");


  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
  std::cout << "Total Throughput: " << (sink1->GetTotalRx ()*8)/(1000000*SimTime) << " Mbps" << std::endl;
}
