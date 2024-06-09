#pragma once

#include <inttypes.h>
#include <string>

namespace pol4b {

#pragma pack(push, 1)
/**
 * @brief The IPv4Addr class.
 *
 * This class represents an IPv4 address and provides functionalities
 * to manipulate and convert IPv4 addresses.
 */
class IPv4Addr {
public:
  /**
   * @brief Construct a new IPv4Addr object with default values.
   */
  IPv4Addr();

  /**
   * @brief Construct a new IPv4Addr object from a 32-bit integer.
   *
   * @param addr 32-bit integer representing the IPv4 address.
   * @param is_network Indicates if the address is in network byte order.
   */
  IPv4Addr(uint32_t addr, bool is_network=false);

  /**
   * @brief Construct a new IPv4Addr object from a string.
   *
   * @param addr String representation of the IPv4 address.
   */
  IPv4Addr(std::string addr);

  /**
   * @brief Convert the IPv4 address to a string.
   *
   * @return String representation of the IPv4 address.
   */
  operator std::string() const;

  /**
   * @brief Convert the IPv4 address to a 32-bit integer.
   *
   * @return 32-bit integer representation of the IPv4 address.
   */
  operator uint32_t() const;

  /**
   * @brief Increase the IPv4 address by a given number.
   *
   * @param n Integer value to add to the IPv4 address.
   * @return IPv4Addr The resulting IPv4 address after addition.
   */
  IPv4Addr operator+=(int n);

  /**
   * @brief Pre-increment the IPv4 address.
   *
   * @return IPv4Addr& Reference to the incremented IPv4 address.
   */
  IPv4Addr &operator++();

  /**
   * @brief Post-increment the IPv4 address.
   *
   * @param int Dummy parameter to differentiate post-increment from pre-increment.
   * @return IPv4Addr The IPv4 address before incrementing.
   */
  IPv4Addr operator++(int);

  /**
   * @brief Access the byte at the given index.
   *
   * @param index Index of the byte to access.
   * @return uint8_t The byte at the given index.
   */
  uint8_t operator[](int index) const;

  /**
   * @brief Copy the IPv4 address to the given destination array.
   *
   * @param dest Pointer to the destination array.
   * @param network Indicates if the IPv4 address should be copied in network byte order.
   */
  void copy(uint8_t *dest, bool network=false) const;

  /**
   * @brief Convert the IPv4 address to host byte order.
   */
  void to_host_byte_order();

  /**
   * @brief Convert the IPv4 address to network byte order.
   */
  void to_network_byte_order();

private:
  uint32_t data; // 32-bit integer to store the IPv4 address.
};
#pragma pack(pop)

};
