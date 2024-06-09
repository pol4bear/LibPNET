#include "l2/arp.h"
#include "netinfomanager.h"
#include <iostream>

using namespace std;
using namespace pol4b;

int main() {
  auto route_info = NetInfoManager::instance().get_default_routeinfo();
  if (route_info.first.empty())
    throw runtime_error("You are not connected to the internet.");
  auto ip_range = NetInfoManager::instance().get_ip_range(route_info.first);
  if (ip_range.first == 0)
    throw runtime_error("Failed to get IP range of the interface.");
  IPv4Addr start = ip_range.first;
  IPv4Addr end = start + 10;
  if (end > ip_range.second)
    end = ip_range.second;
  for (IPv4Addr ip = start; ip < end; ip++) {
    cout << "Scanning " << (string)ip << endl;
    try {
      MACAddr mac = ARP::get_mac_addr(ip);
      cout << (string)ip << " is at " << (string)mac << endl;
    }
    catch (const exception &e) {
      cerr << e.what() << endl;
    }
  }
  return 0;
}
