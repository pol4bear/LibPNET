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
  if (len != 6)
    throw invalid_argument("Invalid MAC address");
  memcpy(data, addr, 6);
}

MACAddr::MACAddr() {
  memset(data, 0, sizeof(data));
}

MACAddr::MACAddr(uint8_t *addr, size_t len, bool is_network) {
  init(addr, len);
  if (is_network)
    to_host_byte_order();
}

MACAddr::MACAddr(uint64_t addr) {
  init((uint8_t*)&addr, 6);
}

MACAddr::MACAddr(const char *addr) {
  string tmp_addr = addr;
  tmp_addr.erase(remove(tmp_addr.begin(), tmp_addr.end(), '-'), tmp_addr.end());
  tmp_addr.erase(remove(tmp_addr.begin(), tmp_addr.end(), ':'), tmp_addr.end());
  if (tmp_addr.size() != 12)
    throw invalid_argument("Invalid MAC address.");

  size_t pos;
  uint64_t converted = stoul(tmp_addr, &pos, 16);
  if (pos != 12)
    throw invalid_argument("Invalid MAC address.");
  init((uint8_t*)&converted, 6);
}

MACAddr::operator std::string() const {
  stringstream result;
  for (int i = 0; i < 5; i++)
    result << hex << uppercase << setw(2) << setfill('0') << (uint32_t)(*this)[i] << ":";
  result << hex << uppercase << setw(2) << setfill('0') << (uint32_t)(*this)[5];
  return result.str();
}

MACAddr::operator uint64_t() const {
  uint64_t result = 0;
  memcpy(&result, data, 6);
  return result;
}

 uint8_t MACAddr::operator[](int index) const {
  if (index < 0 || index > 5)
    return 0;
  uint64_t *addr = (uint64_t*)data;
  return  (*addr >> (40 - index * 8)) & 0xFF;
}

void MACAddr::copy(uint8_t *dest, bool network) const {
  if (dest == nullptr)
    return;
  memcpy(dest, this, sizeof(MACAddr));
  if (network)
    ((MACAddr*)dest)->to_network_byte_order();

}

void MACAddr::to_host_byte_order() {
  if (htonl(1) != 1) {
    uint8_t addr[6];
    for (int i = 0; i < 6; i++)
      addr[i] = data[5 - i];
    memcpy(data, addr, 6);
  }
};

void MACAddr::to_network_byte_order() {
  uint64_t addr = 0;
  memcpy(&addr, data, 6);
  addr = htobe64(addr) >> 16;
  memcpy(data, &addr, 6);
}

}
