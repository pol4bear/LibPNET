#include "l2/l2.h"
#include "netinfomanager.h"
#include <stdexcept>
#include <atomic>
#include <algorithm>
#include <thread>
#include <chrono>
#include <unordered_set>
#include <sys/socket.h>
#include <sys/select.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <memory.h>
#include <fcntl.h>

using namespace std;

namespace pol4b {

ARP::ARP() = default;

MACAddr ARP::get_mac_addr(IPv4Addr ip_addr, int timeout) {
  // The MAC address of target device.
  MACAddr mac_addr;

  // Get the optimal network interface to the target devices.
  auto route_info = NetInfoManager::instance().get_best_routeinfo(ip_addr);
  if (route_info.first.empty())
    throw invalid_argument("Failed to get route to IP address.");

  // Get the network information of the network interface.
  auto if_info = NetInfoManager::instance().get_netinfo(route_info.first);
  if (if_info == nullptr)
    throw runtime_error("Failed to get interface information.");
  else if (if_info->ip == ip_addr)
    return if_info->mac;
  else if (((uint32_t)if_info->ip & if_info->mask) != ((uint32_t)ip_addr & if_info->mask))
    throw invalid_argument("The IP address is not in same network.");

  // Create the raw socket to send and receive ARP packets.
  int if_index = NetInfoManager::instance().get_interface_index(route_info.first);
  if (if_index == -1)
    throw runtime_error("Failed to get interface index.");
  const IPv4Addr &my_ip = (uint32_t)route_info.second->prefsrc == 0 ?  if_info->ip : route_info.second->prefsrc;
  int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
  if (sock < 0)
    throw runtime_error("Failed to create socket.");

  atomic<bool> stop_thread(false);
  thread send_thread;
  try {
    // Set nonblock flag to the raw socket.
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
      throw runtime_error("Failed to get socket flags.");
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
      throw runtime_error("Failed to set socket to non-blocking mode.");

    // Bind the raw socket.
    sockaddr_ll sa;
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_ifindex = if_index;
    if (::bind(sock, (sockaddr*)&sa, sizeof(sa)) < 0)
      throw runtime_error("Failed to bind socket.");

    // Generate the ARP request packet.
    ARP request = ARP::make_packet(if_info->mac, 0xFFFFFFFFFFFF,
                     ARPHeader::Operation::Request, if_info->mac, my_ip,
                     (uint64_t)0, ip_addr);

    // Send the ARP requests every 0.1 seconds.
    auto send_arp_request = [&mac_addr, &sock, &request, &sa, &timeout, &stop_thread]() {
      while (!stop_thread) {
        if (sendto(sock, &request, sizeof(request), 0, (sockaddr*)&sa, sizeof(sa)) < 0)
          break;
        this_thread::sleep_for(chrono::milliseconds(100));
      }
    };
    send_thread = thread(send_arp_request);

    // Set timeout of the raw socket;
    timeval timeout_value;
    timeout_value.tv_sec = timeout;
    timeout_value.tv_usec = 0;

    // Receive the ARP response.
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);
    ARP reply;
    bool received = false;
    while (true) {
      int retval = select(sock + 1, &read_fds, NULL, NULL, &timeout_value);
      if (retval == -1) {
        stop_thread = true;
        send_thread.join();
        throw runtime_error("select(): " + string(strerror(errno)));
      } else if (retval == 0) {
        break;
      }

      auto n = recvfrom(sock, &reply, sizeof(reply), 0, NULL, NULL);
      if (n < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
          continue;
        stop_thread = true;
        send_thread.join();
        throw runtime_error("recvfrom(): " + string(strerror(errno)));
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

    // Cleanup.
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

void ARP::get_mac_addr(std::list<IPv4Addr> ip_addrs, std::function<void(IPv4Addr, MACAddr)> callback, int batch, int retries) {
    // Validate input arguments
    if (ip_addrs.size() < 1)
        throw std::invalid_argument("IP list must have at least 1 item.");
    else if (batch < 1)
        throw std::invalid_argument("Batch size must be bigger than 1.");
    else if (retries < 1)
        throw std::invalid_argument("Retry count must be bigger than 1.");

    // Get the optimal network interface to the target devices
    auto route_info = NetInfoManager::instance().get_best_routeinfo(ip_addrs.front());
    if (route_info.first.empty())
        throw std::invalid_argument("Failed to get route to IP address.");

    // Get the network information of the network interface
    auto if_info = NetInfoManager::instance().get_netinfo(route_info.first);
    if (if_info == nullptr)
        throw std::runtime_error("Failed to get interface information.");

    // Get the network interface index
    int if_index = NetInfoManager::instance().get_interface_index(route_info.first);
    if (if_index == -1)
        throw std::runtime_error("Failed to get interface index.");

    const IPv4Addr &my_ip = (uint32_t)route_info.second->prefsrc == 0 ? if_info->ip : route_info.second->prefsrc;

    // Create the raw socket to send and receive ARP packets
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sock < 0)
        throw std::runtime_error("Failed to create socket.");

    // Bind the raw socket
    sockaddr_ll sa;
    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_ifindex = if_index;
    if (::bind(sock, (sockaddr*)&sa, sizeof(sa)) < 0) {
        close(sock);
        throw std::runtime_error("Failed to bind socket.");
    }

    // Data structure to store IP addresses to be processed
    std::unordered_set<uint32_t> tmp_ip_addrs;
    std::mutex ip_set_mutex; // Mutex to protect access to tmp_ip_addrs
    std::atomic<bool> stop_thread(false);
    std::thread receive_thread;

    // Lambda function to receive ARP responses
    auto receive_arp_response = [&]() {
        while (!stop_thread) {
            ARP reply;
            auto n = recvfrom(sock, &reply, sizeof(reply), 0, NULL, NULL);
            if (n < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                    continue;
                throw std::runtime_error("recvfrom(): " + std::string(strerror(errno)));
            }
            if (reply.eth_hdr.ether_type == htons((uint16_t)EthernetHeader::Ethertype::ARP) &&
                reply.arp_hdr.operation == htons((uint16_t)ARPHeader::Operation::Reply) &&
                ntohl(reply.arp_hdr.target_protocol_address) == my_ip) {
                IPv4Addr ip = ntohl(reply.arp_hdr.sender_protocol_address);
                if (tmp_ip_addrs.find(ip) != tmp_ip_addrs.end()) {
                  MACAddr mac;
                  reply.arp_hdr.sender_hardware_address.copy((uint8_t*)&mac);
                  mac.to_host_byte_order();
                  {
                      std::lock_guard<std::mutex> lock(ip_set_mutex);
                      tmp_ip_addrs.erase(ip);
                  }
                  callback(ip, mac);
                }
            }
        }
    };

    // Start the thread to receive ARP responses
    receive_thread = std::thread(receive_arp_response);

    try {
        while (!ip_addrs.empty()) {
            auto it = ip_addrs.begin();
            // Process IP addresses in batches
            for (int i = 0; i < batch && it != ip_addrs.end(); i++) {
                {
                    std::lock_guard<std::mutex> lock(ip_set_mutex);
                    tmp_ip_addrs.insert(*it);
                }
                it = ip_addrs.erase(it);
            }

            // Send ARP requests
            for (int i = 0; i < retries; i++) {
                std::lock_guard<std::mutex> lock(ip_set_mutex);
                for (auto ip_addr : tmp_ip_addrs) {
                    ARP request = ARP::make_packet(if_info->mac, 0xFFFFFFFFFFFF,
                        ARPHeader::Operation::Request, if_info->mac, my_ip,
                        (uint64_t)0, ip_addr);
                    if (sendto(sock, &request, sizeof(request), 0, (sockaddr*)&sa, sizeof(sa)) < 0)
                        throw std::runtime_error("Failed to send ARP request: " + std::string(strerror(errno)));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // Wait until either all responses are received or a timeout occurs
            auto start_time = std::chrono::steady_clock::now();
            auto timeout = 500 - 100 * retries;
            auto timeout_duration = std::chrono::milliseconds(timeout > 0 ? timeout : 0);
            while (true) {
                {
                    std::lock_guard<std::mutex> lock(ip_set_mutex);
                    if (tmp_ip_addrs.empty()) {
                        break;
                    }
                }
                auto elapsed_time = std::chrono::steady_clock::now() - start_time;
                if (elapsed_time >= timeout_duration) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            {
                std::lock_guard<std::mutex> lock(ip_set_mutex);
                tmp_ip_addrs.clear();
            }
        }

        // Cleanup
        stop_thread = true;
        receive_thread.join();
        close(sock);
        callback(0, 0);
    } catch (...) {
        stop_thread = true;
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
        close(sock);
        throw;
    }
}

ARP ARP::make_packet(MACAddr source_mac, MACAddr dest_mac,
                     ARPHeader::Operation operation, MACAddr sender_mac, IPv4Addr sender_ip,
                     MACAddr target_mac, IPv4Addr target_ip)
{
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
