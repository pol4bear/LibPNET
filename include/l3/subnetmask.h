#pragma once

#include "ipv4.h"

namespace pol4b {

#pragma pack(push, 1)
/**
 * @brief The SubnetMask class.
 *
 * This class represents a subnet mask and provides functionalities
 * to create and manipulate subnet masks, including conversion to and from CIDR notation.
 */
class SubnetMask : public IPv4Addr {
public:
  /**
   * @brief Construct a new SubnetMask object with default values.
   */
  SubnetMask();

  /**
   * @brief Construct a new SubnetMask object from a 32-bit integer.
   *
   * @param addr 32-bit integer representing the subnet mask.
   */
  SubnetMask(uint32_t addr);

  /**
   * @brief Construct a new SubnetMask object from a C-string.
   *
   * @param addr C-string representation of the subnet mask.
   */
  SubnetMask(const char *addr);

  /**
   * @brief Create a SubnetMask object from a CIDR prefix length.
   *
   * @param cidr The CIDR prefix length.
   * @return SubnetMask The resulting SubnetMask object.
   */
  static SubnetMask from_cidr(int cidr);

  /**
   * @brief Convert the SubnetMask object to a CIDR prefix length.
   *
   * @return int The CIDR prefix length.
   */
  int to_cidr() const;
};

#pragma pack(pop)

};
