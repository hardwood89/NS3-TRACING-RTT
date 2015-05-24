/*****************************************************************************/
// This program started life as tcp-variants-comparison.cc provided in the NS3
// Suite. tcp-variants-comparison.cc was written BY:
//            Justin P. Rohrer, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
//            Siddharth Gangadhar <siddharth@ittc.ku.edu>
//
// All the modified done to this code is performed by Alan Lin. The implentation
// of this code is aimed to used existing tracing methodology and to add a few
// more trace sources to give access to the actual sampled RTT, delta between the
// sampled rtt and estimated RTT.
//
// NOTE: The "RTT" trace in the existing suite is mislabelled. The value returned
//       is actually the output of the rtt estimate. Modifications to
//       tcp-socket-base.c/h and rtt-estimator.c/h were made to make available the
//       trace for sampled RTT and the delta.
//
// TOPOLOGY:
//
//                (access)        (shared)        (access)
//         source1--------- GW-left-----GW-Right--------sink1
//         source2---------                     --------sink2
//                (access2)                        (access2)
//                    .                                .
//                    .                                .
//         sourceN---------                     --------sinkN
//                (access1/2)        (shared)        (access1/2)
//
// Can create N source and sink pair and Flows between each pair.
// Each pair has their own flow thro a shair pair of gateways that are used
// by other nodes in the topology.
//
// There are two sets of access deleys, one is assigned to odd flows
// the other is assigned to even flows
//
/*****************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "../src/internet/model/tcp-socket-base-1.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DumbbellDifferingFlows");

/*****************************************************************************/
/*Initialation and pointers to trace streams                                 */
/*****************************************************************************/
bool firstCwnd = true;
bool firstSshThr = true;
bool firstRtt = true;
bool firstRto = true;
bool firstRttVar = true;
bool firstRealRtt = true;
bool firstDelta = true;

bool firstRtt_2 = true;

Ptr<OutputStreamWrapper> cWndStream;
Ptr<OutputStreamWrapper> ssThreshStream;
Ptr<OutputStreamWrapper> rttStream;
Ptr<OutputStreamWrapper> rtoStream;

Ptr<OutputStreamWrapper> rttvarStream;
Ptr<OutputStreamWrapper> rrttStream;
Ptr<OutputStreamWrapper> deltaStream;

Ptr<OutputStreamWrapper> rttStream_2;

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
  *rttStream->GetStream () << m_nodeID << " " << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
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
  *rttvarStream->GetStream () <<  Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
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
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
}

static void
TraceSsThresh (std::string ssthresh_tr_file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream = ascii.CreateFileStream (ssthresh_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SsThreshTracer));
}

static void
TraceRtt (std::string rtt_tr_file_name)
{
  AsciiTraceHelper ascii;
  rttStream = ascii.CreateFileStream (rtt_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/*/RTT", MakeCallback (&RttTracer));
}

static void
TraceRto (std::string rto_tr_file_name)
{
  AsciiTraceHelper ascii;
  rtoStream = ascii.CreateFileStream (rto_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTO", MakeCallback (&RtoTracer));
}
static void
TraceRealRtt (std::string rrtt_tr_file_name)
{
  AsciiTraceHelper ascii;
  rrttStream = ascii.CreateFileStream (rrtt_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/realRTT", MakeCallback (&RealRttTracer));
}
static void
TraceRttVar (std::string rttvar_tr_file_name)
{
  AsciiTraceHelper ascii;
  rttvarStream = ascii.CreateFileStream (rttvar_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTTvar", MakeCallback (&RttVarTracer));
}
static void
TraceDelta (std::string delta_tr_file_name)
{
  AsciiTraceHelper ascii;
  deltaStream = ascii.CreateFileStream (delta_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/3/$ns3::TcpL4Protocol/SocketList/*/Delta", MakeCallback (&DeltaTracer));
}




/**********EXPERIMENTAL SECOND RTT TRACE***********************************/

static void
RttTracer_2 (Time oldval, Time newval)
{
  if (firstRtt)
    {
      *rttStream_2->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRtt_2 = false;
    }
  *rttStream_2->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}


static void
TraceRtt_2 (std::string rtt_tr_file_name_2)
{
  AsciiTraceHelper ascii;
  rttStream_2 = ascii.CreateFileStream (rtt_tr_file_name_2.c_str ());
  Config::ConnectWithoutContext ("/NodeList/3/$ns3::TcpL4Protocol/SocketList/*/RTT", MakeCallback (&RttTracer_2));
}

/**********************************************/



int main (int argc, char *argv[])
{

  // User may find it convenient to enable logging
  LogComponentEnable("DumbbellDifferingFlows", LOG_LEVEL_ALL);
  LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
  //LogComponentEnable("RttEstimator", LOG_LEVEL_INFO);
  //LogComponentEnable("DropTailQueue", LOG_LEVEL_ALL);


  //
  // Set up some default values for the simulation.
  //

  /*Default TCP flavor*/
  //NS_LOG_INFO ("DEFAULT SETTINGS");
  //NS_LOG_INFO ("TcpWestwood");
  std::string transport_prot = "TcpWestwood";
  /*error rate/probability */
  //NS_LOG_INFO ("error probabiltiy 0.0");
  double error_p = 0.0;

  /*Default TCP flavor*/
  NS_LOG_INFO ("Bottleneck Link: 10Mbps , 35ms delay");
  std::string shared_bandwidth = "10Mbps";
  std::string shared_delay = "35ms";

  NS_LOG_INFO ("Access Links: 2Mbps , 175ms delay");
  std::string access_bandwidth = "2Mbps";
  std::string access_delay = "175ms";


  NS_LOG_INFO ("Access2 Links: 2Mbps , 45ms delay");
  std::string access_bandwidth2 = "2Mbps";
  std::string access_delay2 = "45ms";



  /*Tracing, trace file names*/
  bool tracing = true;
  std::string tr_file_name = "";
  std::string cwnd_tr_file_name = "";
  std::string ssthresh_tr_file_name = "";
  std::string rtt_tr_file_name = "";
  std::string rto_tr_file_name = "";
  std::string rttvar_tr_file_name = "";
  std::string rrtt_tr_file_name = "";
  std::string delta_tr_file_name = "";

  std::string rtt_tr_file_name_2 = "rtt_2.trace";


  /*Simulation parameters*/
  double data_mbytes = 1;
  uint32_t mtu_bytes = 400;
  uint16_t num_flows = 1;
  float duration = 30;
  uint32_t run = 0;
  bool flow_monitor = true;

  SeedManager::SetSeed (1);
  SeedManager::SetRun (run);

  // Set the simulation start and stop time
  float start_time = 0.1;
  float stop_time = start_time + duration;


  CommandLine cmd;
  cmd.AddValue ("transport_prot", "Transport protocol to use: TcpTahoe, TcpReno, TcpNewReno, TcpWestwood, TcpWestwoodPlus ", transport_prot);
  cmd.AddValue ("error_p", "Packet error rate", error_p);
  cmd.AddValue ("shared_bandwidth", "Bottleneck bandwidth", shared_bandwidth);
  cmd.AddValue ("access_bandwidth", "Access link bandwidth", access_bandwidth);
  cmd.AddValue ("delay", "Access link delay", access_delay);
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.AddValue ("tr_name", "Name of output trace file", tr_file_name);
  cmd.AddValue ("cwnd_tr_name", "Name of output trace file", cwnd_tr_file_name);
  cmd.AddValue ("ssthresh_tr_name", "Name of output trace file", ssthresh_tr_file_name);
  cmd.AddValue ("rtt_tr_name", "Name of output trace file for ESIMATED RTT", rtt_tr_file_name);
  cmd.AddValue ("rto_tr_name", "Name of output trace file", rto_tr_file_name);
  cmd.AddValue ("rttvar_tr_name", "Name of output trace file", rttvar_tr_file_name);
  cmd.AddValue ("rrtt_tr_name", "Name of output trace file for REAL RTT", rrtt_tr_file_name);
  cmd.AddValue ("delta_tr_name", "Name of output trace file", delta_tr_file_name);
  cmd.AddValue ("data", "Number of Megabytes of data to transmit", data_mbytes);
  cmd.AddValue ("mtu", "Size of IP packets to send in bytes", mtu_bytes);
  cmd.AddValue ("num_flows", "Number of flows", num_flows);
  cmd.AddValue ("duration", "Time to allow flows to run in seconds", duration);
  cmd.AddValue ("run", "Run index (for setting repeatable seeds)", run);
  cmd.AddValue ("flow_monitor", "Enable flow monitor", flow_monitor);
  cmd.Parse (argc, argv);



  // Calculate the ADU size
  Header* temp_header = new Ipv4Header ();
  uint32_t ip_header = temp_header->GetSerializedSize ();
  //NS_LOG_LOGIC ("IP Header size is: " << ip_header);
  delete temp_header;
  temp_header = new TcpHeader ();
  uint32_t tcp_header = temp_header->GetSerializedSize ();
  //NS_LOG_LOGIC ("TCP Header size is: " << tcp_header);
  delete temp_header;
  uint32_t tcp_adu_size = mtu_bytes - (ip_header + tcp_header);
  //NS_LOG_LOGIC ("TCP ADU size is: " << tcp_adu_size);




  // Select TCP variant
  if (transport_prot.compare ("TcpTahoe") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpTahoe::GetTypeId ()));
    }
  else if (transport_prot.compare ("TcpReno") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpReno::GetTypeId ()));
    }
  else if (transport_prot.compare ("TcpNewReno") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpNewReno::GetTypeId ()));
    }
  else if (transport_prot.compare ("TcpWestwood") == 0)
    { // the default protocol type in ns3::TcpWestwood is WESTWOOD
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
    }
  else if (transport_prot.compare ("TcpWestwoodPlus") == 0)
    {
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
      Config::SetDefault ("ns3::TcpWestwood::FilterType", EnumValue (TcpWestwood::TUSTIN));
    }
  else
    {
      NS_LOG_DEBUG ("Invalid TCP version");
      exit (1);
    }


    // Create gateways, sources, and sinks
    NodeContainer leftGate;
    leftGate.Create (1);
    NS_LOG_INFO ("Number of Nodes After left Gate: " << NodeList::GetNNodes());


    NodeContainer rightGate;
    rightGate.Create (1);
    NS_LOG_INFO ("Number of Nodes After Right Gate: " << NodeList::GetNNodes());


    NodeContainer sources;
    sources.Create (num_flows);

    NS_LOG_INFO ("Totoal Number of Nodes After Sources: " << NodeList::GetNNodes());

    NodeContainer sinks;
    sinks.Create (num_flows);

    NS_LOG_INFO ("Total Number of Flows: " << num_flows);
    NS_LOG_INFO ("Total Number of Nodes After Sinks: " << NodeList::GetNNodes());
    Ptr< Node > tmp = sources.Get (0);
    NS_LOG_INFO ("First Source Node Index: " << tmp->GetId());
    tmp = sinks.Get (0);
    NS_LOG_INFO ("First Sink Node Index: " << tmp->GetId());
    NS_LOG_INFO ("TCP Flavor: " << transport_prot);


    // Configure the error model
    // Here we use RateErrorModel with packet error rate
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
    uv->SetStream (50);
    RateErrorModel error_model;
    error_model.SetRandomVariable (uv);
    error_model.SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
    error_model.SetRate (error_p);



    PointToPointHelper BottleNeckLink;
    BottleNeckLink.SetDeviceAttribute ("DataRate", StringValue (shared_bandwidth));
    BottleNeckLink.SetChannelAttribute ("Delay", StringValue (shared_delay));
    BottleNeckLink.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (&error_model));

    NS_LOG_INFO ("Install internet stack on all nodes.");
    InternetStackHelper stack;
    stack.InstallAll ();

    NetDeviceContainer gates;
    gates = BottleNeckLink.Install (leftGate.Get (0),rightGate.Get (0));


    Ipv4AddressHelper address;
    address.SetBase ("10.0.0.0", "255.255.0.0");

    Ipv4InterfaceContainer backbone = address.Assign(gates);

    // Configure the sources and sinks net devices
    // and the channels between the sources/sinks and the gateways
    PointToPointHelper LocalLinkL;
    PointToPointHelper LocalLinkR;



    LocalLinkL.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
    LocalLinkL.SetChannelAttribute ("Delay", StringValue (access_delay));
    LocalLinkR.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
    LocalLinkR.SetChannelAttribute ("Delay", StringValue (access_delay));
    Ipv4InterfaceContainer sink_interfaces;

    for (int i = 0; i < num_flows; i++)
      {

        if ((i%2) == 0) {

          LocalLinkL.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
          LocalLinkL.SetChannelAttribute ("Delay", StringValue (access_delay));
          LocalLinkR.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
          LocalLinkR.SetChannelAttribute ("Delay", StringValue (access_delay));



          NetDeviceContainer devices;
          devices = LocalLinkL.Install (sources.Get (i), leftGate.Get (0));

          address.NewNetwork ();
          Ipv4InterfaceContainer interfaces = address.Assign (devices);


          devices = LocalLinkR.Install (rightGate.Get (0), sinks.Get (i));
          address.NewNetwork ();
          interfaces = address.Assign (devices);
          sink_interfaces.Add (interfaces.Get (1));


        } else {
          LocalLinkL.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth2));
          LocalLinkL.SetChannelAttribute ("Delay", StringValue (access_delay2));
          LocalLinkR.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth2));
          LocalLinkR.SetChannelAttribute ("Delay", StringValue (access_delay2));



          NetDeviceContainer devices;
          devices = LocalLinkL.Install (sources.Get (i), leftGate.Get (0));

          address.NewNetwork ();
          Ipv4InterfaceContainer interfaces = address.Assign (devices);


          devices = LocalLinkR.Install (rightGate.Get (0), sinks.Get (i));
          address.NewNetwork ();
          interfaces = address.Assign (devices);
          sink_interfaces.Add (interfaces.Get (1));
        }

      }





      NS_LOG_INFO ("Initialize Global Routing.");
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      uint16_t port = 50000;
      Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);

      for (uint16_t i = 0; i < sources.GetN (); i++)
        {
          AddressValue remoteAddress (InetSocketAddress (sink_interfaces.GetAddress (i, 0), port));

          if (transport_prot.compare ("TcpTahoe") == 0
              || transport_prot.compare ("TcpReno") == 0
              || transport_prot.compare ("TcpNewReno") == 0
              || transport_prot.compare ("TcpWestwood") == 0
              || transport_prot.compare ("TcpWestwoodPlus") == 0)
            {
              Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcp_adu_size));
              BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
              ftp.SetAttribute ("Remote", remoteAddress);
              ftp.SetAttribute ("SendSize", UintegerValue (tcp_adu_size));
              ftp.SetAttribute ("MaxBytes", UintegerValue (int(data_mbytes * 1000000)));

              ApplicationContainer sourceApp = ftp.Install (sources.Get (i));
              sourceApp.Start (Seconds (start_time * i));
              sourceApp.Stop (Seconds (stop_time - 3));

              sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
              ApplicationContainer sinkApp = sinkHelper.Install (sinks);
              sinkApp.Start (Seconds (start_time * i));
              sinkApp.Stop (Seconds (stop_time));
            }
          else
            {
              NS_LOG_DEBUG ("Invalid transport protocol " << transport_prot << " specified");
              exit (1);
            }
        }


          // Set up tracing if enabled
          if (tracing)
            {
              double trace_start=0.5;
              if (tr_file_name.compare ("") != 0)
                {
                  std::ofstream ascii;
                  Ptr<OutputStreamWrapper> ascii_wrap;
                  ascii.open (tr_file_name.c_str ());
                  ascii_wrap = new OutputStreamWrapper (tr_file_name.c_str (), std::ios::out);
                  stack.EnableAsciiIpv4All (ascii_wrap);
                }

              if (cwnd_tr_file_name.compare ("") != 0)
                {
                  Simulator::Schedule (Seconds (trace_start), &TraceCwnd, cwnd_tr_file_name);
                }

              if (ssthresh_tr_file_name.compare ("") != 0)
                {
                  Simulator::Schedule (Seconds (trace_start), &TraceSsThresh, ssthresh_tr_file_name);
                }
              /*****estimate RTT**************************************************************/
              if (rtt_tr_file_name.compare ("") != 0)
                {
                  Simulator::Schedule (Seconds (trace_start), &TraceRtt, rtt_tr_file_name);
                }

              if (rto_tr_file_name.compare ("") != 0)
                {
                  Simulator::Schedule (Seconds (trace_start), &TraceRto, rto_tr_file_name);
                }

  		        /*****RTT VARIANCE**************************************************************/
              if (rttvar_tr_file_name.compare ("") != 0)
                {
                  Simulator::Schedule (Seconds (trace_start), &TraceRttVar, rttvar_tr_file_name);
                }
              /*****REAL RTT**************************************************************/
              if (rrtt_tr_file_name.compare ("") != 0)
                {
                  Simulator::Schedule (Seconds (trace_start), &TraceRealRtt, rrtt_tr_file_name);
                }

                /*****Delta**************************************************************/
              if (delta_tr_file_name.compare ("") != 0)
                {
                  Simulator::Schedule (Seconds (trace_start), &TraceDelta, delta_tr_file_name);
                }

                /*****estimate RTT**************************************************************/
                if (rtt_tr_file_name_2.compare ("") != 0)
                  {
                    Simulator::Schedule (Seconds (trace_start), &TraceRtt_2, rtt_tr_file_name_2);
                  }

            }

          BottleNeckLink.EnablePcapAll ("DumbbellDifferingFlows", true);
          LocalLinkL.EnablePcapAll ("DumbbellDifferingFlows", true);
          LocalLinkR.EnablePcapAll ("DumbbellDifferingFlows", true);

          // Flow monitor
          FlowMonitorHelper flowHelper;
          if (flow_monitor)
            {
              flowHelper.InstallAll ();
            }

          Simulator::Stop (Seconds (stop_time));
          Simulator::Run ();

          if (flow_monitor)
            {
              flowHelper.SerializeToXmlFile ("DumbbellDifferingFlows.flowmonitor", true, true);
            }

          Simulator::Destroy ();
          return 0;
        }
