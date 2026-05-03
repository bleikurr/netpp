#ifndef __NETPP_ADDRESS_HPP
#define __NETPP_ADDRESS_HPP

#include <cstdint>
#include <netdb.h>
#include <optional>
#include <string>

#include <sys/socket.h>
namespace netpp::address {
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

  struct sockaddr *sockaddr();
  socklen_t addrlen();
  std::string_view ip();
  std::string_view name();
};

typedef struct {
  char *dns;
  char *ip;
  struct sockaddr_storage addr;
  socklen_t addrlen;
  bool reverse;
} Record;

Record *dns_lookup(char *address);
void dns_free(Record *res);
} // namespace netpp::address

#endif
