#include "l3/ipv4.h"
#include <memory.h>
#include <stdexcept>
#include <sstream>
#include <arpa/inet.h>

using namespace std;

namespace pol4b {

IPv4Addr::IPv4Addr() {
  memset(&data, 0, sizeof(data));
}

IPv4Addr::IPv4Addr(uint32_t addr, bool is_network) {
  data = addr;
  if (is_network)
    to_host_byte_order();
}

IPv4Addr::IPv4Addr(const char *addr) {
  if (addr == nullptr)
    return;
  int addr_len = strlen(addr);
  uint32_t tmp = 0;
  if (addr_len < 7 || addr_len > 15)
    throw invalid_argument("Address length must be between 7 and 15 characters.");
  char *tmp_addr = strdup(addr);
  if (tmp_addr == nullptr)
    throw bad_alloc();
  char *token = strtok(tmp_addr, ".");
  int token_count = 0;
  while (token != nullptr && token_count < 4) {
    int num = atoi(token);
    if (num < 0 || num > 255)
      throw invalid_argument("Address must be between 0.0.0.0 and 255.255.255.255.");
    tmp <<= 8;
    tmp |= num;
    token_count++;
    token = strtok(nullptr, ".");
  }
  free(tmp_addr);
  if (token_count != 4)
    throw invalid_argument("Invalid IP address.");
  data = tmp;
}

IPv4Addr::operator std::string() const {
  stringstream result;
  result << (uint32_t)(*this)[0] << "." << (uint32_t)(*this)[1] << "." <<
    (uint32_t)(*this)[2] << "." << (uint32_t)(*this)[3];
  return result.str();
}

IPv4Addr::operator uint32_t() const {
  return data;
}

uint8_t IPv4Addr::operator[](int index) const {
  if (index < 0 || index > 3)
    return 0;
  return (data >> (24 - index * 8)) & 0xFF;
}
void IPv4Addr::copy(uint8_t *dest, bool network) const {
  if (dest == nullptr)
    return;
  memcpy(dest, this, sizeof(IPv4Addr));
  if (network)
    ((IPv4Addr*)dest)->to_network_byte_order();
}

void IPv4Addr::to_host_byte_order() {
  data = ntohl(data);
};

void IPv4Addr::to_network_byte_order() {
  data = htonl(data);
}

}
