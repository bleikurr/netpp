#include "netpp/address.hpp"

#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <stdexcept>
namespace netpp::address {

Address::Address(std::string address, std::string ip, int family,
                 struct sockaddr_storage addr, size_t addrlen)
    : m_record(std::move(address)), m_ip(std::move(ip)), m_port(0),
      m_addr(std::move(addr)), m_addrlen(addrlen), m_addrfamily(family) {}

std::optional<Address> Address::forward_dns_lookup(std::string address) {
  constexpr size_t STRLEN = 64;

  struct addrinfo *ainfo, *rp, *valid = nullptr;
  int ret = getaddrinfo(address.c_str(), nullptr, nullptr, &ainfo);
  if (ret != 0) {
    std::cerr << "Address not found: " << address << std::endl;
    return std::nullopt;
  }

  rp = ainfo;
  while (rp != nullptr) {
    if (rp->ai_family == AF_INET || rp->ai_family == AF_INET6) {
      valid = rp;
      break;
    }
    rp = rp->ai_next;
  }

  if (valid == nullptr) {
    std::cerr << "Address not found: " << address << std::endl;
    freeaddrinfo(ainfo);
    return std::nullopt;
  }

  char ip_str[STRLEN] = {0};
  struct sockaddr_storage addr{};

  socklen_t addrlen = valid->ai_addrlen;
  int family = valid->ai_family;
  std::memcpy(&addr, valid->ai_addr, addrlen);
  if (valid->ai_family == AF_INET) {
    struct sockaddr_in *addr_in = reinterpret_cast<struct sockaddr_in *>(&addr);
    inet_ntop(valid->ai_family, &(addr_in->sin_addr), ip_str, STRLEN);

  } else if (valid->ai_family == AF_INET6) {
    struct sockaddr_in6 *addr_in =
        reinterpret_cast<struct sockaddr_in6 *>(&addr);
    inet_ntop(valid->ai_family, &(addr_in->sin6_addr), ip_str, STRLEN);
  }

  freeaddrinfo(ainfo);

  return Address(std::move(address), ip_str, family, std::move(addr), addrlen);
}

Address Address::reverse_dns_lookup4(std::string ip_addr, in_addr *iaddr) {
  char dns[64] = {0};

  struct sockaddr_storage addr{};
  struct sockaddr_in *in_addr = reinterpret_cast<sockaddr_in *>(&addr);
  in_addr->sin_family = AF_INET;
  in_addr->sin_addr = *iaddr;

  if (getnameinfo((struct sockaddr *)&addr, sizeof(addr), dns, 64, nullptr, 0,
                  0) == 0) {
    return Address(dns, std::move(ip_addr), AF_INET, std::move(addr),
                   sizeof(*in_addr));
  }

  std::string not_found = ip_addr;
  return Address(std::move(not_found), std::move(ip_addr), AF_INET,
                 std::move(addr), sizeof(*in_addr));
}

Address Address::reverse_dns_lookup6(std::string ip_addr, in6_addr *iaddr) {
  char dns[64] = {0};

  struct sockaddr_storage addr{};
  struct sockaddr_in6 *in_addr = reinterpret_cast<sockaddr_in6 *>(&addr);
  in_addr->sin6_family = AF_INET6;
  in_addr->sin6_addr = *iaddr;

  if (getnameinfo((struct sockaddr *)&addr, sizeof(addr), dns, 64, nullptr, 0,
                  0) == 0) {
    return Address(dns, std::move(ip_addr), AF_INET6, std::move(addr),
                   sizeof(*in_addr));
  }

  std::string not_found = ip_addr;
  return Address(std::move(not_found), std::move(ip_addr), AF_INET6,
                 std::move(addr), sizeof(*in_addr));
}

std::optional<Address> Address::get_address(std::string address) {
  struct in_addr iaddr;
  struct in6_addr i6addr;
  if (inet_pton(AF_INET, address.c_str(), &iaddr) == 1) {
    return reverse_dns_lookup4(address, &iaddr);
  } else if (inet_pton(AF_INET6, address.c_str(), &i6addr) == 1) {
    return reverse_dns_lookup6(address, &i6addr);
  } else {
    return forward_dns_lookup(address);
  }
}

Address Address::empty_address() {
  struct sockaddr_storage addr{};
  return Address("", "", 0, std::move(addr), SOCKADDR_MAX_SIZE);
}

struct sockaddr *Address::p_sockaddr() {
  return reinterpret_cast<struct sockaddr *>(&m_addr);
}

socklen_t *Address::p_addrlen() {
  return reinterpret_cast<socklen_t *>(&m_addrlen);
}
socklen_t Address::addrlen() { return m_addrlen; }

std::string_view Address::name() { return m_record; }

std::string_view Address::ip() { return m_ip; }

void Address::parse_sockaddr() {
  constexpr size_t STRLEN = 64;
  char ip_str[64] = {0};

  m_addrfamily = m_addr.ss_family;

  if (m_addrfamily == AF_INET) {
    struct sockaddr_in *addr_in =
        reinterpret_cast<struct sockaddr_in *>(&m_addr);
    inet_ntop(m_addrfamily, &(addr_in->sin_addr), ip_str, STRLEN);

  } else if (m_addrfamily == AF_INET6) {
    struct sockaddr_in6 *addr_in =
        reinterpret_cast<struct sockaddr_in6 *>(&m_addr);
    inet_ntop(m_addrfamily, &(addr_in->sin6_addr), ip_str, STRLEN);
  } else {
    throw std::logic_error("Received from unsupported address family");
  }

  m_ip = ip_str;
}

void Address::reset() {
  m_addrlen = SOCKADDR_MAX_SIZE;
  m_record.clear();
  m_ip.clear();
  m_port = 0;
  m_addrfamily = 0;
}

} // namespace netpp::address
