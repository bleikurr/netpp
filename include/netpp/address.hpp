#ifndef __NETPP_ADDRESS_HPP
#define __NETPP_ADDRESS_HPP

#include <cstdint>
#include <netdb.h>
#include <optional>
#include <string>

#include <sys/socket.h>
namespace netpp::address {
inline constexpr std::size_t SOCKADDR_MAX_SIZE =
    sizeof(struct sockaddr_storage);

class Address {
private:
  std::string m_record;
  std::string m_ip;
  uint16_t m_port;
  struct sockaddr_storage m_addr;
  size_t m_addrlen;
  int m_addrfamily;

  Address(std::string address, std::string ip, int family,
          struct sockaddr_storage addr, size_t addrlen);

  static std::optional<Address> forward_dns_lookup(std::string address);
  static Address reverse_dns_lookup4(std::string ip_addr, in_addr *iaddr);
  static Address reverse_dns_lookup6(std::string ip_addr, in6_addr *iaddr);

public:
  Address() = delete;
  static std::optional<Address> get_address(std::string address);
  static Address empty_address();
  struct sockaddr *p_sockaddr();

  socklen_t *p_addrlen();
  socklen_t addrlen();
  std::string_view ip();
  void parse_sockaddr();
  std::string_view name();
  void reset();
};

} // namespace netpp::address

#endif
