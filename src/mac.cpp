#include "l2/mac.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <memory.h>
#include <iomanip>
#include <arpa/inet.h>

using namespace std;

namespace pol4b {

void MACAddr::init(uint8_t *addr, size_t len) {
  // Check if the length is 6 bytes (valid MAC address length)
  if (len != 6)
    throw invalid_argument("Invalid MAC address length.");

  // Copy the address into the data array
  memcpy(data, addr, 6);
}

MACAddr::MACAddr() {
  // Initialize the MAC address to 00:00:00:00:00:00
  memset(data, 0, sizeof(data));
}

MACAddr::MACAddr(uint8_t *addr, size_t len, bool is_network) {
  // Initialize the MAC address with the given address and length
  init(addr, len);

  // Convert to host byte order if the address is in network byte order
  if (is_network)
    to_host_byte_order();
}

MACAddr::MACAddr(uint64_t addr) {
  // Initialize the MAC address with the lower 6 bytes of the given 64-bit integer
  init((uint8_t*)&addr, 6);
}

MACAddr::MACAddr(string addr) {
  // Remove any '-' or ':' from the address string
  string tmp_addr = addr;
  tmp_addr.erase(remove(tmp_addr.begin(), tmp_addr.end(), '-'), tmp_addr.end());
  tmp_addr.erase(remove(tmp_addr.begin(), tmp_addr.end(), ':'), tmp_addr.end());

  // Check if the address string has exactly 12 characters (valid MAC address length)
  if (tmp_addr.size() != 12)
    throw invalid_argument("Invalid MAC address.");

  // Convert the address string from hexadecimal to a 64-bit integer
  size_t pos;
  uint64_t converted = stoul(tmp_addr, &pos, 16);

  // Verify that the entire string was converted
  if (pos != 12)
    throw invalid_argument("Invalid MAC address.");

  // Initialize the MAC address with the converted value
  init((uint8_t*)&converted, 6);
}

MACAddr::operator std::string() const {
  stringstream result;
  // Convert each byte to a hexadecimal string and append it to the result
  for (int i = 0; i < 5; i++)
    result << hex << uppercase << setw(2) << setfill('0') << (uint32_t)(*this)[i] << ":";

  // Append the last byte without a trailing ':'
  result << hex << uppercase << setw(2) << setfill('0') << (uint32_t)(*this)[5];
  return result.str();
}

MACAddr::operator uint64_t() const {
  uint64_t result = 0;
  // Copy the MAC address data into a 64-bit integer
  memcpy(&result, data, 6);
  return result;
}

MACAddr MACAddr::operator+=(int n) {
  // Convert the MAC address to a 64-bit integer, add n, and convert back to MACAddr
  uint64_t mac = *this;
  mac += n;
  *this = MACAddr(mac);
  return *this;
}

MACAddr &MACAddr::operator++() {
  // Pre-increment the MAC address by 1
  *this += 1;
  return *this;
}

MACAddr MACAddr::operator++(int) {
  // Post-increment the MAC address by 1
  MACAddr tmp = *this;
  ++*this;
  return tmp;
}

uint8_t MACAddr::operator[](int index) const {
  // Check if the index is within valid range
  if (index < 0 || index > 5)
    return 0;

  // Extract the byte at the specified index
  uint64_t *addr = (uint64_t*)data;
  return  (*addr >> (40 - index * 8)) & 0xFF;
}

void MACAddr::copy(uint8_t *dest, bool network) const {
  if (dest == nullptr)
    return;

  // Copy the MAC address to the destination array
  memcpy(dest, this, sizeof(MACAddr));

  // Convert to network byte order if requested
  if (network)
    ((MACAddr*)dest)->to_network_byte_order();
}

void MACAddr::to_host_byte_order() {
  uint64_t addr = 0;
  // Copy the MAC address data into a 64-bit integer
  memcpy(&addr, data, 6);

  // Convert to host byte order
  addr = be64toh(addr << 16);

  // Copy the converted bytes back to the data array
  memcpy(data, &addr, 6);
}

void MACAddr::to_network_byte_order() {
  uint64_t addr = 0;
  // Copy the MAC address data into a 64-bit integer
  memcpy(&addr, data, 6);

  // Convert to network byte order
  addr = htobe64(addr) >> 16;

  // Copy the converted bytes back to the data array
  memcpy(data, &addr, 6);
}

}
