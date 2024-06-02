#include "l3/subnetmask.h"

namespace pol4b {

SubnetMask::SubnetMask() : IPv4Addr() {}

SubnetMask::SubnetMask(const char *addr) : IPv4Addr(addr) {}

SubnetMask::SubnetMask(uint32_t addr) : IPv4Addr(addr) {}

SubnetMask SubnetMask::from_cidr(int cidr) {
  return SubnetMask(0xFFFFFFFF << (32 - cidr));
}

int SubnetMask::to_cidr() const {
  uint32_t data = (uint32_t)*this;
  if (data == 0)
    return 0;
  int zero_count = 0;
  for (; (data & 1) == 0; zero_count++)
    data >>= 1;
  return 32 - zero_count;
}

};
