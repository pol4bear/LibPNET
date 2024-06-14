#pragma once

#include "ether.h"
#include "mac.h"
#include "../l3/ipv4.h"
#include "../netinfo.h"
#include <list>
#include <functional>

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
   * @brief Retrieves the MAC address for a given IPv4 address using ARP.
   *
   * This function sends ARP requests to a specified IP address and waits for the ARP reply to retrieve the MAC address.
   * The function will keep sending ARP requests until an ARP reply is received or the specified timeout is reached.
   *
   * @param ip_addr The IPv4 address for which the MAC address is to be retrieved.
   * @param timeout The timeout period (in seconds) to wait for the ARP reply.
   * @return The MAC address associated with the specified IPv4 address.
   *
   * @throws std::invalid_argument if the IP address is not reachable or not in the same network.
   * @throws std::runtime_error if there is a failure in retrieving network interface information, creating or binding the socket, sending/receiving ARP packets, or if the ARP reply is not received within the timeout period.
   */
  static MACAddr get_mac_addr(IPv4Addr ip_addr, int timeout=1);

  /**
   * @brief Sends ARP requests to a list of IP addresses and retrieves their MAC addresses.
   *
   * This function sends ARP requests to a list of IP addresses and invokes a callback function
   * with the IP address and its corresponding MAC address upon receiving an ARP reply.
   *
   * @param ip_addrs A list of IP addresses to retrieve MAC addresses for. The list must contain at least one item.
   * @param callback A callback function to invoke with the IP address and its corresponding MAC address.
   * @param batch The number of IP addresses to process in each batch. Must be greater than 0. Default is 50.
   * @param retries The number of times to retry sending ARP requests for each batch. Must be greater than 0. Default is 3.
   *
   * @throws std::invalid_argument if the IP list is empty, batch size is less than 1, or retry count is less than 1.
   * @throws std::runtime_error if there is a failure in retrieving network interface information, creating or binding the socket, or sending/receiving ARP packets.
   */
  static void get_mac_addr(std::list<IPv4Addr> ip_addrs, std::function<void(IPv4Addr, MACAddr)> callback, int batch=50, int retries=3);
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
