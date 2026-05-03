#include "netpp/socket.hpp"

#include <netdb.h>
#include <stdexcept>

#include <system_error>
#include <unistd.h>
namespace netpp::sockets {

namespace {
int map_family_to_c_family(Family family) {
  switch (family) {
  case IP4:
    return AF_INET;
  case IP6:
    return AF_INET6;
  case UNIX:
    return AF_UNIX;
  default:
    throw std::invalid_argument("Invalid socket family");
  }
}

int map_type_to_c_type(Type type) {
  switch (type) {
  case DATAGRAM:
    return SOCK_DGRAM;
  case STREAM:
    return SOCK_STREAM;
  case RAW:
    return SOCK_RAW;
  default:
    throw std::invalid_argument("Invalid socket type");
  }
}

int map_protocol_to_c_protocol(const std::string &protocol) {
  auto proto = getprotobyname(protocol.c_str());
  if (proto == nullptr) {
    throw std::invalid_argument("Protocol not found: " + protocol);
  }
  return proto->p_proto;
}
} // namespace

Socket::Socket(int sockfd, int family, int type, int protocol)
    : m_fd(sockfd), m_family(family), m_type(type), m_protocol(protocol),
      m_address(std::nullopt) {}

Socket::Socket(Socket &&other) noexcept
    : m_fd(other.m_fd), m_family(other.m_family), m_type(other.m_type),
      m_protocol(other.m_protocol), m_address(other.m_address) {
  other.m_fd = -1;
}

Socket &Socket::operator=(Socket &&other) noexcept {
  if (this != &other) { // 1. Don't move to yourself
    if (m_fd != -1)
      close(m_fd); // 2. Close my current socket!

    m_fd = other.m_fd; // 3. Take the new one
    other.m_fd = -1;   // 4. Null out the old one
  }
  return *this;
}

Socket::~Socket() {
  close(m_fd);
  m_fd = -1;
}

Socket Socket::ClientSocket(Family family, Type type,
                            const std::string &protocol) {
  int ifamily = map_family_to_c_family(family);
  int itype = map_type_to_c_type(type);
  int iprotocol = map_protocol_to_c_protocol(protocol);

  int sockfd = socket(ifamily, itype, iprotocol);
  if (sockfd < 0) {
    std::error_code ec(errno, std::system_category());
    throw std::system_error(ec, "Failed to create socket");
  }
  Socket sock(sockfd, ifamily, itype, iprotocol);

  return sock;
}

void Socket::set_address(address::Address address) {
  m_address = std::move(address);
}

void Socket::sendto(const std::vector<std::byte> &data) {
  if (!m_address)
    throw std::logic_error(
        "No address set in 'sendto', did you forget to set address?");
  // TODO Look at passing flags
  ::sendto(m_fd, data.data(), data.size(), 0, m_address->sockaddr(),
           m_address->addrlen());
}

void Socket::recvfrom(std::vector<std::byte> &data) {
  if (!m_address)
    throw std::logic_error(
        "No address set in 'recvfrom', did you forget to set address?");

  struct sockaddr_storage addr{};
  socklen_t addrlen = sizeof(addr);
  // TODO Look at passing flags
  ::recvfrom(m_fd, data.data(), data.capacity(), 0,
             reinterpret_cast<struct sockaddr *>(&addr), &addrlen);
}

int Socket::sockfd() { return m_fd; }

} // namespace netpp::sockets
