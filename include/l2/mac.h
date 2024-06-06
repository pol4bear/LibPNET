#pragma once

#include <inttypes.h>
#include <string>

namespace pol4b {

#pragma pack(push, 1)
class MACAddr {
  public:
    MACAddr();
    MACAddr(uint8_t *addr, size_t len, bool is_network=false);
    MACAddr(uint64_t addr);
    MACAddr(std::string addr);
    operator std::string() const;
    operator uint64_t() const;
    MACAddr operator+=(int n);
    MACAddr &operator++();
    MACAddr operator++(int);
    uint8_t operator[](int index) const;
    void copy(uint8_t *dest, bool network=false) const;
    void to_host_byte_order();
    void to_network_byte_order();

  private:
    uint8_t data[6];
    void init(uint8_t *addr, size_t len);
};
#pragma pack(pop)

};
