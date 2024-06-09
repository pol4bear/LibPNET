#pragma once

#include "mac.h"

namespace pol4b {

#pragma pack(push, 1)
/**
 * @brief The EthernetHeader class.
 *
 * Represents an Ethernet header containing the source MAC address,
 * destination MAC address, and EtherType.
 */
class EthernetHeader {
public:
  /**
   * @brief Construct a new EthernetHeader object with default values.
   */
  EthernetHeader() = default;

  /**
   * @brief The destination MAC address.
   */
  MACAddr destination_mac;

  /**
   * @brief The source MAC address.
   */
  MACAddr source_mac;

  /**
   * @brief The EtherType field indicating the protocol of the encapsulated payload.
   */
  uint16_t ether_type;

  /**
   * @brief Enumeration of common EtherType values.
   */
  enum class Ethertype : uint16_t {
    IPv4 = 0x0800, // EtherType for IPv4
    ARP = 0x0806,  // EtherType for ARP
    IPv6 = 0x86DD  // EtherType for IPv6
  };
};

#pragma pack(pop)

}
