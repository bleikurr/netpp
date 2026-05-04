#ifndef __NETPP_SOCKETS_HPP
#define __NETPP_SOCKETS_HPP

#include "netpp/address.hpp"
#include <string>
#include <sys/socket.h>
#include <vector>

namespace netpp::sockets {
enum Family { IP4, IP6, UNIX };
enum Type { DATAGRAM, STREAM, RAW };
enum Protocol { UDP, TCP, ICMP };

class Socket {
private:
  Socket(int sockfd, int domain, int type, int protocol);
  int m_fd;
  int m_family;
  int m_type;
  int m_protocol;

  std::optional<address::Address> m_address;

public:
  static Socket ClientSocket(Family domain, Type type,
                             const std::string &protocol);

  ~Socket();

  // No allow copy
  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

  // Allow move
  Socket(Socket &&other) noexcept;
  Socket &operator=(Socket &&other) noexcept;

  // TODO Breyta DNSResult í eitthvað meira generic address
  // og til að taka í sitt eigið project
  void set_address(address::Address address);
  void sendto(const std::vector<std::byte> &data);

  /*
   * @source will be reset when passed into this function
   */
  void recvfrom(std::vector<std::byte> &data, address::Address &source);
  int sockfd();
};
} // namespace netpp::sockets

#endif
