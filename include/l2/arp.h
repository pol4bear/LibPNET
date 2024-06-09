#pragma once

#include "ether.h"
#include "mac.h"
#include "../l3/ipv4.h"
#include "../netinfo.h"

namespace pol4b {

#pragma pack(push, 1)
/**
 * @brief The ARPHeader class.
 *
 * Represents an ARP (Address Resolution Protocol) header containing all necessary fields
 * for ARP requests and replies.
 */
class ARPHeader {
public:
  /**
   * @brief Construct a new ARPHeader object with default values.
   */
  ARPHeader() = default;

  /**
   * @brief Hardware type (e.g., Ethernet).
   */
  uint16_t hardware_type;

  /**
   * @brief Protocol type (e.g., IPv4).
   */
  uint16_t protocol_type;

  /**
   * @brief Length of the hardware address.
   */
  uint8_t hardware_address_length;

  /**
   * @brief Length of the protocol address.
   */
  uint8_t protocol_address_length;

  /**
   * @brief Operation (e.g., request or reply).
   */
  uint16_t operation;

  /**
   * @brief The sender's hardware address (MAC address).
   */
  MACAddr sender_hardware_address;

  /**
   * @brief The sender's protocol address (IP address).
   */
  IPv4Addr sender_protocol_address;

  /**
   * @brief The target's hardware address (MAC address).
   */
  MACAddr target_hardware_address;

  /**
   * @brief The target's protocol address (IP address).
   */
  IPv4Addr target_protocol_address;

  /**
   * @brief Enumeration of common hardware types.
   */
  enum class HardwareType : uint16_t {
    Ethernet = 1 /**< Hardware type for Ethernet. */
  };

  /**
   * @brief Enumeration of common protocol types.
   */
  enum class ProtocolType : uint16_t {
    IPv4 = 0x0800 /**< Protocol type for IPv4. */
  };

  /**
   * @brief Enumeration of ARP operations.
   */
  enum class Operation : uint16_t {
    Request = 1, /**< ARP request operation. */
    Reply = 2  /**< ARP reply operation. */
  };
};
#pragma pack(pop)

/**
 * @brief The ARP packet class.
 *
 * This class provides functionalities to generate, send, and receive ARP packets.
 */
class ARP {
public:
  ARP();
  EthernetHeader eth_hdr;
  ARPHeader arp_hdr;

  /**
   * @brief Get the MAC address of the target device.
   *
   * @param ip_addr The IP address of the target device.
   * @param timeout Timeout(seconds) to wait ARP response.
   * @return MACAddr The MAC address of the target device.
   */
  static MACAddr get_mac_addr(IPv4Addr ip_addr, int timeout=1);
  /**
   * @brief Generate ARP packet.
   *
   * @param source_mac The source MAC address of the Ethernet header.
   * @param dest_mac The destination MAC address of the Ethernet header.
   * @param operation ARP operation(Request or Response).
   * @param sender_mac The sender MAC address of the ARP header.
   * @param sender_ip The sender IP address of the ARP header.
   * @param target_mac The target MAC address of the ARP header.
   * @param target_ip The target IP address of the ARP header.
   * @return ARP
   */
  static ARP make_packet(MACAddr source_mac, MACAddr dest_mac,
  ARPHeader::Operation operation, MACAddr sender_mac, IPv4Addr sender_ip,
  MACAddr target_mac, IPv4Addr target_ip);
};

}
