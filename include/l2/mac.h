#pragma once

#include <inttypes.h>
#include <string>

namespace pol4b {

#pragma pack(push, 1)
/**
 * @brief The MACAddr class.
 *
 * This class represents a MAC address and provides functionalities to manipulate and convert MAC addresses.
 */
class MACAddr {
public:
  /**
   * @brief Construct a new MACAddr object with default values.
   */
  MACAddr();

  /**
   * @brief Construct a new MACAddr object from a byte array.
   *
   * @param addr Pointer to the byte array containing the MAC address.
   * @param len Length of the byte array.
   * @param is_network Indicates if the byte array is in network byte order.
   */
  MACAddr(uint8_t *addr, size_t len, bool is_network=false);

  /**
   * @brief Construct a new MACAddr object from a 64-bit integer.
   *
   * @param addr 64-bit integer representing the MAC address.
   */
  MACAddr(uint64_t addr);

  /**
   * @brief Construct a new MACAddr object from a string.
   *
   * @param addr String representing the MAC address.
   */
  MACAddr(std::string addr);

  /**
   * @brief Convert the MAC address to a string.
   *
   * @return String representation of the MAC address.
   */
  operator std::string() const;

  /**
   * @brief Convert the MAC address to a 64-bit integer.
   *
   * @return 64-bit integer representation of the MAC address.
   */
  operator uint64_t() const;

  /**
   * @brief Increase the MAC address by a given number.
   *
   * @param n Integer value to add to the MAC address.
   * @return MACAddr The resulting MAC address after addition.
   */
  MACAddr operator+=(int n);

  /**
   * @brief Pre-increment the MAC address.
   *
   * @return MACAddr& Reference to the incremented MAC address.
   */
  MACAddr &operator++();

  /**
   * @brief Post-increment the MAC address.
   *
   * @param int Dummy parameter to differentiate post-increment from pre-increment.
   * @return MACAddr The MAC address before incrementing.
   */
  MACAddr operator++(int);

  /**
   * @brief Access the byte at the given index.
   *
   * @param index Index of the byte to access.
   * @return uint8_t The byte at the given index.
   */
  uint8_t operator[](int index) const;

  /**
   * @brief Copy the MAC address to the given destination array.
   *
   * @param dest Pointer to the destination array.
   * @param network Indicates if the MAC address should be copied in network byte order.
   */
  void copy(uint8_t *dest, bool network=false) const;

  /**
   * @brief Convert the MAC address to host byte order.
   */
  void to_host_byte_order();

  /**
   * @brief Convert the MAC address to network byte order.
   */
  void to_network_byte_order();

private:
  uint8_t data[6]; // Byte array to store the MAC address.

  /**
   * @brief Initialize the MAC address from a byte array.
   *
   * @param addr Pointer to the byte array containing the MAC address.
   * @param len Length of the byte array.
   */
  void init(uint8_t *addr, size_t len);
};
#pragma pack(pop)

};
