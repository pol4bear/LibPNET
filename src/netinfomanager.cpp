#include "netinfomanager.h"
#include "l2/arp.h"
#include <stdexcept>
#include <memory.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <fcntl.h>
#include <iostream>
#include <cstdint>

using namespace std;

namespace pol4b {

NetInfoManager::NetInfoManager() {}
NetInfoManager::~NetInfoManager() {}

int NetInfoManager::send_netlink_request(int sock, int type, uint8_t table, int flags) {
    struct {
        nlmsghdr nlh;
        rtmsg rtm;
    } request;

    memset(&request, 0, sizeof(request));
    request.nlh.nlmsg_len = sizeof(request);
    request.nlh.nlmsg_type = type;
    request.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP | flags;
    request.nlh.nlmsg_seq = 1;
    request.rtm.rtm_family = AF_INET;
    request.rtm.rtm_table = table;

    return send(sock, &request, sizeof(request), 0);
}

NetInfoManager &NetInfoManager::instance() {
  static NetInfoManager net_info_manager;
  return net_info_manager;
}

void NetInfoManager::load_netinfo() {
  lock_guard<mutex> guard(this->interfaces_mutex);
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock < 0)
    throw runtime_error("Failed to create netlink socket.");
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  sockaddr_nl addr;
  memset(&addr, 0 , sizeof(addr));
  addr.nl_family = AF_NETLINK;

  char buffer[8192];
  int len = 0;
  map<int, NetInfo> interface_map;
  send_netlink_request(sock, RTM_GETLINK);
  while ((len = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
    for (nlmsghdr *nh = (nlmsghdr*)buffer; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
      if (nh->nlmsg_type == NLMSG_DONE)
        break;
      else if (nh->nlmsg_type == RTM_NEWLINK) {
        ifinfomsg *iface = (ifinfomsg *)NLMSG_DATA(nh);
        rtattr *attr = (rtattr *) IFLA_RTA(iface);
        int length = nh->nlmsg_len - NLMSG_LENGTH(sizeof(*iface));
        for (; RTA_OK(attr, length); attr = RTA_NEXT(attr, length)) {
            if (attr->rta_type == IFLA_IFNAME) {
              auto if_name = (char*)RTA_DATA(attr);
              interface_name[iface->ifi_index] = if_name;
              interface_index[if_name] = iface->ifi_index;
            }
            else if (attr->rta_type == IFLA_ADDRESS) {
              interface_map[iface->ifi_index].mac = MACAddr((uint8_t*)RTA_DATA(attr), 6, true);
            }
        }
      }
    }
  }
  send_netlink_request(sock, RTM_GETADDR);
  while ((len = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
    for (nlmsghdr *nh = (nlmsghdr*)buffer; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
      if (nh->nlmsg_type == NLMSG_DONE)
        break;
      else if (nh->nlmsg_type == RTM_NEWADDR) {
        ifaddrmsg *ifa = (ifaddrmsg*)NLMSG_DATA(nh);
        rtattr *attr = (rtattr*)IFA_RTA(ifa);
        int ifa_len = IFA_PAYLOAD(nh);
        for (; RTA_OK(attr, ifa_len); attr = RTA_NEXT(attr, ifa_len)) {
          if (attr->rta_type == IFA_LOCAL) {
            interface_map[ifa->ifa_index].ip = IPv4Addr(ntohl(*(uint32_t*)RTA_DATA(attr)));
            break;
          }
        }
        interface_map[ifa->ifa_index].mask = SubnetMask::from_cidr((int)ifa->ifa_prefixlen);
      }
    }
  }

  interfaces.clear();
  for (const auto &interface : interface_map)
    interfaces[interface_name[interface.first]] = interface.second;
}

void NetInfoManager::load_routeinfo() {
  lock_guard<mutex> guard(this->routes_mutex);
  if (interface_name.size() < 1)
    load_netinfo();

  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock < 0)
    throw runtime_error("Failed to create netlink socket.");
  int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);
  sockaddr_nl addr;
  memset(&addr, 0 , sizeof(addr));
  addr.nl_family = AF_NETLINK;

  char buffer[8192];
  int len = 0;
  send_netlink_request(sock, RTM_GETROUTE);
  while ((len = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
    for (nlmsghdr *nh = (nlmsghdr*)buffer; NLMSG_OK(nh, len); nh = NLMSG_NEXT(nh, len)) {
      if (nh->nlmsg_type == NLMSG_DONE)
        break;
      else if (nh->nlmsg_type == RTM_NEWROUTE) {
        rtmsg *rtm = (rtmsg *)NLMSG_DATA(nh);
        rtattr *attr = (rtattr *)RTM_RTA(rtm);
        int length = RTM_PAYLOAD(nh);
        uint32_t tmp = 0;
        string ifname = "";
        RouteInfo route_info;
        for (; RTA_OK(attr, length); attr = RTA_NEXT(attr, length)) {
          switch(attr->rta_type) {
          case RTA_GATEWAY:
            memcpy(&tmp, RTA_DATA(attr), sizeof(tmp));
            route_info.gateway = IPv4Addr(ntohl(tmp));
            break;
          case RTA_DST:
            memcpy(&tmp, RTA_DATA(attr), sizeof(tmp));
            route_info.destination = IPv4Addr(ntohl(tmp));
            break;
          case RTA_PREFSRC:
            memcpy(&tmp, RTA_DATA(attr), sizeof(tmp));
            route_info.prefsrc = IPv4Addr(ntohl(tmp));
            break;
          case RTA_PRIORITY:
            memcpy(&tmp, RTA_DATA(attr), sizeof(tmp));
            route_info.metric = ntohl(tmp);
            break;
          case RTA_OIF:
            memcpy(&tmp, RTA_DATA(attr), sizeof(tmp));
            ifname = interface_name[tmp];
            break;
          }
          route_info.mask = SubnetMask(rtm->rtm_dst_len);
        }
        if (route_info.prefsrc != 0 || route_info.gateway != 0)
          routes[ifname].push_back(route_info);
      }
    }
  }
}

const NetInfoMap &NetInfoManager::get_all_netinfo(bool reload) {
  if (reload || interfaces.size() < 1)
    load_netinfo();
  lock_guard<mutex> guard(this->interfaces_mutex);
  return interfaces;
}

const RouteInfoMap &NetInfoManager::get_all_routeinfo(bool reload) {
  if (reload || routes.size() < 1)
    load_routeinfo();
  lock_guard<mutex> guard(this->routes_mutex);
  return routes;
}

const NetInfo *NetInfoManager::get_netinfo(string name) {
  if (name.empty())
    return nullptr;
  else if (interfaces.size() < 1)
    load_netinfo();
  lock_guard<mutex> guard(this->interfaces_mutex);
  auto netinfo = interfaces.find(name);
  if (netinfo == interfaces.end())
    return nullptr;
  return &netinfo->second;
}

const IPv4Addr *NetInfoManager::get_gateway_ip(string name) {
  IPv4Addr *gateway_ip = nullptr;
  if (name.empty())
    return gateway_ip;
  else if (routes.size() < 1)
    load_routeinfo();
  lock_guard<mutex> guard(this->routes_mutex);
  for (auto &route : routes[name]) {
    if (route.gateway != 0) {
      gateway_ip = &route.gateway;
      break;
    }
  }
  return gateway_ip;
}

NetInfo NetInfoManager::get_gateway_info(std::string name) {
  NetInfo gateway_info;
  auto gateway_ip = NetInfoManager::instance().get_gateway_ip(name);
  if (gateway_ip == nullptr)
    throw invalid_argument("Failed to get gateway IP address.");
  gateway_info.ip = *gateway_ip;
  gateway_info.mac = ARP::get_mac_addr(*gateway_ip);
  return gateway_info;
}

pair<IPv4Addr, IPv4Addr> NetInfoManager::get_ip_range(IPv4Addr ip, SubnetMask mask) {
  return make_pair((ip & mask) + 1, (ip | ~mask) - 1);
}

pair<IPv4Addr, IPv4Addr> NetInfoManager::get_ip_range(string name, SubnetMask maximum_mask) {
  if (name.empty())
    throw invalid_argument("Empty interface name.");
  else if (interfaces.size() < 1)
    load_routeinfo();
  lock_guard<mutex> guard(this->routes_mutex);
  auto interface = interfaces.find(name);
  if (interface == interfaces.end())
    throw invalid_argument("Invalid interface name.");
  SubnetMask mask = interface->second.mask > maximum_mask ? interface->second.mask : maximum_mask;
  return get_ip_range(interface->second.ip, mask);
}

RouteInfoWithName NetInfoManager::get_best_routeinfo(IPv4Addr destination) {
  string ifname = "";
  const RouteInfo *best_route = nullptr;
  int longest_prefix = -1;
  if (routes.size() < 1)
    load_routeinfo();
  lock_guard<mutex> guard(this->routes_mutex);
  for (auto &ifroute : routes) {
    for (auto &route : ifroute.second) {
      if ((destination & route.mask) == route.destination) {
        int prefix_len = SubnetMask(route.mask).to_cidr();
        if (prefix_len > longest_prefix || (prefix_len == longest_prefix &&
          route.metric < best_route->metric)) {
          longest_prefix = prefix_len;
          ifname = ifroute.first;
          best_route = &route;
        }
      }
    }
  }
  return make_pair(ifname, best_route);
}

RouteInfoWithName NetInfoManager::get_default_routeinfo() {
  string ifname = "";
  const RouteInfo *default_route = nullptr;
  if (routes.size() < 1)
    load_routeinfo();
  lock_guard<mutex> guard(this->routes_mutex);
  for (auto &ifroute : routes) {
    for (auto &route : ifroute.second) {
      if ((uint32_t)route.destination == 0 && (uint32_t)route.mask == 0) {
        ifname = ifroute.first;
        default_route = &route;
      }
    }
  }
  return make_pair(ifname, default_route);
}

string NetInfoManager::get_interface_name(int index) {
  if (interface_name.size() < 1)
    load_netinfo();
  lock_guard<mutex> guard(this->interfaces_mutex);
  auto name = interface_name.find(index);
  if (name == interface_name.end())
    return "";
  return name->second;
}

int NetInfoManager::get_interface_index(std::string name) {
  if (interface_index.size() < 1)
    load_netinfo();
  lock_guard<mutex> guard(this->interfaces_mutex);
  auto index = interface_index.find(name);
  if (index == interface_index.end())
    return -1;
  return index->second;
}

};
