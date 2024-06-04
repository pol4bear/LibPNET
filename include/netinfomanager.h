#pragma once

#include "netinfo.h"
#include "routeinfo.h"
#include <vector>
#include <map>
#include <unordered_map>
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
  RouteInfoMap routes;
  int send_netlink_request(int sock, int type, uint8_t table=RT_TABLE_MAIN, int flags=0);


public:
  static NetInfoManager& instance();
  void load_netinfo();
  void load_routeinfo();
  const NetInfoMap &get_all_netinfo(bool reload=false);
  const RouteInfoMap &get_all_routeinfo(bool reload=false);
  const NetInfo *get_netinfo(std::string name);
  const IPv4Addr *get_gateway_ip(std::string name);
  RouteInfoWithName get_best_routeinfo(IPv4Addr destination="8.8.8.8");
  RouteInfoWithName get_default_routeinfo();
  std::string get_interface_name(int index);
  int get_interface_index(std::string name);
};

};
