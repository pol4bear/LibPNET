#include "l2/l2.h"
#include "netinfomanager.h"
#include <stdexcept>
#include <atomic>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <memory.h>
#include <fcntl.h>

using namespace std;

namespace pol4b {

ARP::ARP() = default;

MACAddr ARP::get_mac_addr(IPv4Addr ip_addr, int timeout) {
    MACAddr mac_addr;
    auto route_info = NetInfoManager::instance().get_best_routeinfo(ip_addr);
    if (route_info.second == nullptr)
        throw invalid_argument("Failed to get route to IP address.");
    auto if_info = NetInfoManager::instance().get_netinfo(route_info.first);
    if (if_info == nullptr)
        throw runtime_error("Failed to get interface information.");
    else if (if_info->ip == ip_addr)
        return if_info->mac;
    else if (((uint32_t)if_info->ip & if_info->mask) != ((uint32_t)ip_addr & if_info->mask))
        throw invalid_argument("The IP address is not in same network.");

    int if_index = NetInfoManager::instance().get_interface_index(route_info.first);
    if (if_index == -1)
        throw runtime_error("Failed to get interface index.");

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sock < 0)
        throw runtime_error("Failed to create socket.");

    atomic<bool> stop_thread(false);
    thread send_thread;

    try {
        int flags = fcntl(sock, F_GETFL, 0);
        if (flags < 0)
            throw runtime_error("Failed to get socket flags.");
        if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
            throw runtime_error("Failed to set socket to non-blocking mode.");

        sockaddr_ll sa;
        memset(&sa, 0, sizeof(sa));
        sa.sll_family = AF_PACKET;
        sa.sll_protocol = htons(ETH_P_ARP);
        sa.sll_ifindex = if_index;
        if (::bind(sock, (sockaddr*)&sa, sizeof(sa)) < 0)
            throw runtime_error("Failed to bind socket.");

        ARP request = ARP::make_packet(if_info->mac, 0xFFFFFFFFFFFF,
                                       ARPHeader::Operation::Request, if_info->mac, route_info.second->prefsrc,
                                       (uint64_t)0, ip_addr);

        auto send_arp_request = [&mac_addr, &sock, &request, &sa, &timeout, &stop_thread]() {
            while (!stop_thread) {
                if (sendto(sock, &request, sizeof(request), 0, (sockaddr*)&sa, sizeof(sa)) < 0)
                    break;
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        };

        send_thread = thread(send_arp_request);

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);

        timeval timeout_value;
        timeout_value.tv_sec = timeout;
        timeout_value.tv_usec = 0;

        ARP reply;
        bool received = false;
        while (true) {
            int retval = select(sock + 1, &read_fds, NULL, NULL, &timeout_value);
            if (retval == -1) {
                stop_thread = true;
                send_thread.join();
                throw runtime_error("select() failed.");
            } else if (retval == 0) {
                break;
            }

            auto n = recvfrom(sock, &reply, sizeof(reply), 0, NULL, NULL);
            if (n < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                    continue;
                stop_thread = true;
                send_thread.join();
                throw runtime_error("recvfrom() failed.");
            }

            if (reply.eth_hdr.ether_type == htons((uint16_t)EthernetHeader::Ethertype::ARP) &&
                reply.arp_hdr.operation == htons((uint16_t)ARPHeader::Operation::Reply) &&
                request.arp_hdr.sender_protocol_address == reply.arp_hdr.target_protocol_address &&
                request.arp_hdr.target_protocol_address == reply.arp_hdr.sender_protocol_address) {
                reply.arp_hdr.sender_hardware_address.copy((uint8_t*)&mac_addr);
                mac_addr.to_host_byte_order();
                received = true;
                break;
            }
        }

        stop_thread = true;
        send_thread.join();
        close(sock);

        if (!received)
            throw runtime_error("Failed to receive ARP reply.");
    } catch (...) {
        stop_thread = true;
        if (send_thread.joinable()) {
            send_thread.join();
        }
        close(sock);
        throw;
    }

    return mac_addr;
}

ARP ARP::make_packet(MACAddr source_mac, MACAddr dest_mac,
  ARPHeader::Operation operation, MACAddr sender_mac, IPv4Addr sender_ip,
  MACAddr target_mac, IPv4Addr target_ip) {
  ARP arp_packet;
  dest_mac.copy((uint8_t*)&arp_packet.eth_hdr.destination_mac, true);
  source_mac.copy((uint8_t*)&arp_packet.eth_hdr.source_mac, true);
  arp_packet.eth_hdr.ether_type = htons((uint16_t)EthernetHeader::Ethertype::ARP);
  arp_packet.arp_hdr.hardware_type = htons((uint16_t)ARPHeader::HardwareType::Ethernet);
  arp_packet.arp_hdr.protocol_type = htons((uint16_t)ARPHeader::ProtocolType::IPv4);
  arp_packet.arp_hdr.hardware_address_length = sizeof(MACAddr);
  arp_packet.arp_hdr.protocol_address_length = sizeof(IPv4Addr);
  arp_packet.arp_hdr.operation = htons((uint16_t)operation);
  sender_mac.copy((uint8_t*)&arp_packet.arp_hdr.sender_hardware_address, true);
  sender_ip.copy((uint8_t*)&arp_packet.arp_hdr.sender_protocol_address, true);
  target_mac.copy((uint8_t*)&arp_packet.arp_hdr.target_hardware_address, true);
  target_ip.copy((uint8_t*)&arp_packet.arp_hdr.target_protocol_address, true);
  return arp_packet;
}

};
