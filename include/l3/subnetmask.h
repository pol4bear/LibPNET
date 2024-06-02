#pragma once

#include "ipv4.h"

namespace pol4b {

#pragma pack(push, 1)
class SubnetMask : public IPv4Addr {
  public:
    SubnetMask();
    SubnetMask(uint32_t addr);
    SubnetMask(const char *addr);
    static SubnetMask from_cidr(int cidr);
    int to_cidr() const;
};
#pragma pack(pop)

};
