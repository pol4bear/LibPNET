#include "l3/subnetmask.h"

namespace pol4b {

// Default constructor: Calls the IPv4Addr default constructor.
SubnetMask::SubnetMask() : IPv4Addr() {}

// Constructor: Initializes SubnetMask object from a string address.
SubnetMask::SubnetMask(const char *addr) : IPv4Addr(addr) {}

// Constructor: Initializes SubnetMask object from a 32-bit integer address.
SubnetMask::SubnetMask(uint32_t addr) : IPv4Addr(addr) {}

// Static method to create a SubnetMask object from a CIDR prefix length.
SubnetMask SubnetMask::from_cidr(int cidr) {
  // Creates a subnet mask by shifting 0xFFFFFFFF to the left by (32 - cidr).
  return SubnetMask(0xFFFFFFFF << (32 - cidr));
}

// Converts the SubnetMask object to a CIDR prefix length.
int SubnetMask::to_cidr() const {
  uint32_t data = (uint32_t)*this;
  if (data == 0)
    return 0;
  int zero_count = 0;
  // Counts the number of trailing zero bits in the subnet mask.
  for (; (data & 1) == 0; zero_count++)
    data >>= 1;
  return 32 - zero_count;
}

};
