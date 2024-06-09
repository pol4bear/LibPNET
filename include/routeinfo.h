#pragma once

#include "l3/l3.h"

namespace pol4b {

/**
 * @brief The RouteInfo class.
 *
 * This class represents routing information, including the gateway, destination,
 * preferred source address, subnet mask, and metric.
 */
class RouteInfo {
public:
  /**
   * @brief Construct a new RouteInfo object with default values.
   */
  RouteInfo() = default;

  /**
   * @brief The gateway IP address for the route.
   */
  IPv4Addr gateway;

  /**
   * @brief The destination IP address for the route.
   */
  IPv4Addr destination;

  /**
   * @brief The preferred source IP address for the route.
   */
  IPv4Addr prefsrc;

  /**
   * @brief The subnet mask for the route.
   */
  SubnetMask mask;

  /**
   * @brief The metric for the route.
   *
   * Metric is used to determine the preference of the route. Lower values indicate higher preference.
   */
  uint32_t metric = 0;
};

};
