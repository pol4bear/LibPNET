#pragma once

#include "netinfo.h"
#include "routeinfo.h"
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <linux/rtnetlink.h>

namespace pol4b {

/**
 * @typedef NetInfoMap
 * @brief Alias for a map storing network information with interface names as keys.
 */
using NetInfoMap = std::unordered_map<std::string, NetInfo>;

/**
 * @typedef RouteInfoMap
 * @brief Alias for a map storing route information with interface names as keys.
 */
using RouteInfoMap = std::unordered_map<std::string, std::vector<RouteInfo>>;

/**
 * @typedef RouteInfoWithName
 * @brief Alias for a pair containing an interface name and a pointer to route information.
 */
using RouteInfoWithName = std::pair<std::string, const RouteInfo*>;

/**
 * @class NetInfoManager
 * @brief Manages network interface and route information.
 */
class NetInfoManager {

private:
  /**
   * @brief Private constructor to enforce singleton pattern.
   */
  NetInfoManager();

  /**
   * @brief Private destructor.
   */
  virtual ~NetInfoManager();

  /**
   * @brief Map of interface indices to interface names.
   */
  std::map<int, std::string> interface_name;

  /**
   * @brief Map of interface names to interface indices.
   */
  std::unordered_map<std::string, int> interface_index;

  /**
   * @brief Map storing network information for each interface.
   */
  NetInfoMap interfaces;

  /**
   * @brief Mutex for protecting access to the interfaces map.
   */
  std::mutex interfaces_mutex;

  /**
   * @brief Map storing route information for each interface.
   */
  RouteInfoMap routes;

  /**
   * @brief Mutex for protecting access to the routes map.
   */
  std::mutex routes_mutex;

  /**
   * @brief Sends a netlink request.
   * @param sock The socket to use for the request.
   * @param type The type of request.
   * @param table The routing table to query (default is RT_TABLE_MAIN).
   * @param flags Additional flags for the request.
   * @return The result of the send operation.
   */
  int send_netlink_request(int sock, int type, uint8_t table=RT_TABLE_MAIN, int flags=0);

public:
  /**
   * @brief Gets the singleton instance of the NetInfoManager.
   * @return A reference to the singleton instance.
   */
  static NetInfoManager& instance();

  /**
   * @brief Loads network interface information.
   */
  void load_netinfo();

  /**
   * @brief Loads route information.
   */
  void load_routeinfo();

  /**
   * @brief Gets all network interface information.
   * @param reload Whether to reload the information.
   * @return A const reference to the map of network information.
   */
  const NetInfoMap &get_all_netinfo(bool reload=false);

  /**
   * @brief Gets all route information.
   * @param reload Whether to reload the information.
   * @return A const reference to the map of route information.
   */
  const RouteInfoMap &get_all_routeinfo(bool reload=false);

  /**
   * @brief Gets network information for a specific interface.
   * @param name The name of the interface.
   * @return A pointer to the network information, or nullptr if not found.
   */
  const NetInfo *get_netinfo(std::string name);

  /**
   * @brief Gets the gateway IP address for a specific interface.
   * @param name The name of the interface.
   * @return A pointer to the gateway IP address, or nullptr if not found.
   */
  const IPv4Addr *get_gateway_ip(std::string name);

  /**
   * @brief Gets the IP range for a given IP and subnet mask.
   * @param ip The IP address.
   * @param mask The subnet mask.
   * @return A pair containing the start and end IP addresses of the range.
   */
  std::pair<IPv4Addr, IPv4Addr> get_ip_range(IPv4Addr ip, SubnetMask mask);

  /**
   * @brief Gets the IP range for a specific interface.
   * @param name The name of the interface.
   * @param maximum_mask The maximum subnet mask (default is an empty subnet mask).
   * @return A pair containing the start and end IP addresses of the range.
   */
  std::pair<IPv4Addr, IPv4Addr> get_ip_range(std::string name, SubnetMask maximum_mask=SubnetMask());

  /**
   * @brief Gets the best route information for a given destination.
   * @param destination The destination IP address (default is 8.8.8.8).
   * @return A pair containing the interface name and the best route information.
   */
  RouteInfoWithName get_best_routeinfo(IPv4Addr destination=std::string("8.8.8.8"));

  /**
   * @brief Gets the default route information.
   * @return A pair containing the interface name and the default route information.
   */
  RouteInfoWithName get_default_routeinfo();

  /**
   * @brief Gets the name of an interface by its index.
   * @param index The index of the interface.
   * @return The name of the interface, or an empty string if not found.
   */
  std::string get_interface_name(int index);

  /**
   * @brief Gets the index of an interface by its name.
   * @param name The name of the interface.
   * @return The index of the interface, or -1 if not found.
   */
  int get_interface_index(std::string name);
};

};
