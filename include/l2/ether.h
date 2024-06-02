#pragma once

#include "mac.h"

namespace pol4b {

#pragma pack(push, 1)
class EthernetHeader {
public:
  EthernetHeader() = default;

  MACAddr destination_mac;
  MACAddr source_mac;
  uint16_t ether_type;

  enum class Ethertype : uint16_t {
    IPv4 = 0x0800,
    ARP = 0x0806,
    IPv6 = 0x86DD
  };
};
#pragma pack(pop)

}
