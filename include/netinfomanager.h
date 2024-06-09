#pragma once

#include "netinfo.h"
#include "routeinfo.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <linux/rtnetlink.h>

namespace pol4b {

using NetInfoMap = std::unordered_map<std::string, NetInfo>;
using RouteInfoMap = std::unordered_map<std::string, std::vector<RouteInfo>>;
using RouteInfoWithName=std::pair<std::string, const RouteInfo*>;

class NetInfoManager {

private:
  NetInfoManager();
  virtual ~NetInfoManager();

  std::map<int, std::string> interface_name;
  std::unordered_map<std::string, int> interface_index;
  NetInfoMap interfaces;
  std::mutex interfaces_mutex;
  RouteInfoMap routes;
  std::mutex routes_mutex;;
  int send_netlink_request(int sock, int type, uint8_t table=RT_TABLE_MAIN, int flags=0);


public:
  static NetInfoManager& instance();
  void load_netinfo();
  void load_routeinfo();
  const NetInfoMap &get_all_netinfo(bool reload=false);
  const RouteInfoMap &get_all_routeinfo(bool reload=false);
  const NetInfo *get_netinfo(std::string name);
  const IPv4Addr *get_gateway_ip(std::string name);
  NetInfo get_gateway_info(std::string name);
  std::pair<IPv4Addr, IPv4Addr> get_ip_range(IPv4Addr ip, SubnetMask mask);
  std::pair<IPv4Addr, IPv4Addr> get_ip_range(std::string name, SubnetMask maximum_mask=SubnetMask());
  RouteInfoWithName get_best_routeinfo(IPv4Addr destination=std::string("8.8.8.8"));
  RouteInfoWithName get_default_routeinfo();
  std::string get_interface_name(int index);
  int get_interface_index(std::string name);
};

};
