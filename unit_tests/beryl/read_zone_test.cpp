#include "beryl/read_zone.hpp"

#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include "beryl/domain_name.hpp"
#include "beryl/record_consumer.hpp"
#include "beryl/record_visitor.hpp"
#include "beryl/resource_record.hpp"

#include "unit_testing/expect_throw_msg_eq.hpp"

using beryl::domain_name;
using beryl::read_zone;
using beryl::record_consumer;
using beryl::resource_record;

namespace {
class visitor_x final : public beryl::record_visitor {
public:
  visitor_x(std::ostream& os) : _os(os), need_space(false) {}
  void visit_record_begin() final { need_space = false; }
  void visit_record_end() final {}
  void visit(beryl::record_class rc) final { _impl(rc); }
  void visit(beryl::record_type rt) final { _impl(rt); }
  void visit(std::uint32_t ui) final { _impl(ui); }
  void visit(const domain_name& str) final { _impl(str); }
  void visit(boost::asio::ip::address_v4 addr) final { _impl(addr); }
  void visit(const boost::asio::ip::address_v6& addr) final { _impl(addr); }

private:
  template <typename T>
  void _impl(T&& arg) {
    _os << (need_space ? " " : "") << std::forward<T>(arg);
    need_space = true;
  }

  std::ostream& _os;
  bool need_space;
};

class consumer_x : public record_consumer {
public:
  void consume_zone_begin() final {}
  void consume_zone_end() final {}
  void consume(domain_name&& name, std::unique_ptr<resource_record> rr) final {
    _records.emplace_back(std::move(name), std::move(rr));
  }

  std::string to_string() const {
    std::ostringstream s;
    visitor_x v(s);
    bool need_newline = false;
    for (const auto& [name, rr] : _records) {
      s << (need_newline ? "\n" : "") << name << " ";
      rr->accept(v);
      need_newline = true;
    }
    return s.str();
  }

private:
  std::vector<std::pair<domain_name, std::unique_ptr<resource_record>>> _records;
};

void expect_invalid_zone(const std::string& zone, const std::string& msg) {
  SCOPED_TRACE(zone);
  std::stringstream s;
  s << zone;
  consumer_x c;
  EXPECT_THROW_MSG_EQ(read_zone(s, c), std::runtime_error, msg.c_str());
}

void expect_zone_eq(const std::string& zone, const std::string& expected) {
  SCOPED_TRACE(zone);
  std::stringstream s;
  s << zone;
  consumer_x c;
  read_zone(s, c);
  EXPECT_EQ(c.to_string(), expected);
}
}  // namespace

TEST(dns_read_zone_test, too_few_or_too_many_tokens_is_not_ok) {
  expect_invalid_zone("lima.mike.",
                      "line 1: too few tokens in the resource record");
  expect_invalid_zone("lima.mike. 3600",
                      "line 1: too few tokens in the resource record");
  expect_invalid_zone("lima.mike. 3600 IN",
                      "line 1: too few tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike.",
      "line 2: too few tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike. 3600",
      "line 2: too few tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike. 3600 IN",
      "line 2: too few tokens in the resource record");
}

TEST(dns_read_zone_test, soa_too_few_or_too_many_tokens_is_not_ok) {
  expect_invalid_zone("lima.mike. 3600 IN SOA",
                      "line 1: too few tokens in the resource record");
  expect_invalid_zone("lima.mike. 3600 IN SOA ns0.lima.mike.",
                      "line 1: too few tokens in the resource record");
  expect_invalid_zone("lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike.",
                      "line 1: too few tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1",
      "line 1: too few tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2",
      "line 1: too few tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3",
      "line 1: too few tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4",
      "line 1: too few tokens in the resource record");
  expect_zone_eq(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      ".mike.lima 3600 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5 6",
      "line 1: too many tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5 6 7",
      "line 1: too many tokens in the resource record");
}

TEST(dns_read_zone_test, ns_too_few_or_too_many_tokens_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike. 600 IN NS",
      "line 2: too few tokens in the resource record");
  expect_zone_eq(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike. 600 IN NS ns0.lima.mike.",
      ".mike.lima 3600 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5\n"
      ".mike.lima 600 IN NS .mike.lima.ns0");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike. 600 IN NS ns0.lima.mike. 42",
      "line 2: too many tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike. 600 IN NS ns0.lima.mike. foo 142",
      "line 2: too many tokens in the resource record");
}

TEST(dns_read_zone_test, cname_too_few_or_too_many_tokens_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN CNAME",
      "line 2: too few tokens in the resource record");
  expect_zone_eq(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN CNAME alpha.lima.mike.",
      ".mike.lima 3600 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5\n"
      ".mike.lima.ns0 600 IN CNAME .mike.lima.alpha");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN CNAME alpha.lima.mike. 42",
      "line 2: too many tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN CNAME alpha.lima.mike. foo 142",
      "line 2: too many tokens in the resource record");
}

TEST(dns_read_zone_test, a_too_few_or_too_many_tokens_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN A",
      "line 2: too few tokens in the resource record");
  expect_zone_eq(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN A 1.2.3.4",
      ".mike.lima 3600 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5\n"
      ".mike.lima.ns0 600 IN A 1.2.3.4");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN A 1.2.3.4 42",
      "line 2: too many tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN A 1.2.3.4 foo 142",
      "line 2: too many tokens in the resource record");
}

TEST(dns_read_zone_test, aaaa_too_few_or_too_many_tokens_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN AAAA",
      "line 2: too few tokens in the resource record");
  expect_zone_eq(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN AAAA 1:2:3:4:c:d:e:f",
      ".mike.lima 3600 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5\n"
      ".mike.lima.ns0 600 IN AAAA 1:2:3:4:c:d:e:f");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN AAAA 1:2:3:4:c:d:e:f 42",
      "line 2: too many tokens in the resource record");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN AAAA 1:2:3:4:c:d:e:f foo 142",
      "line 2: too many tokens in the resource record");
}

TEST(dns_read_zone_test, invalid_domain_is_not_ok) {
  expect_invalid_zone(
      "li#ma-.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: invalid domain name: `li#ma-.mike.`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mi&ke-. admin.lima.mike. 1 2 3 4 5",
      "line 1: invalid domain name: `ns0.lima.mi&ke-.`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. -admin.lima.mike. 1 2 3 4 5",
      "line 1: invalid domain name: `-admin.lima.mike.`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "*lima7.mike. 600 IN NS ns0.lima.mike.",
      "line 2: invalid domain name: `*lima7.mike.`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike. 600 IN NS ns0.lima.-mike.",
      "line 2: invalid domain name: `ns0.lima.-mike.`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.*lima.mike. 600 IN CNAME alpha.lima.mike.",
      "line 2: invalid domain name: `ns0.*lima.mike.`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN CNAME alpha(.lima.mike.",
      "line 2: invalid domain name: `alpha(.lima.mike.`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mik@e. 600 IN A 1.2.3.4",
      "line 2: invalid domain name: `ns0.lima.mik@e.`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.li)ma.mike. 600 IN AAAA 1:2:3:4:c:d:e:f",
      "line 2: invalid domain name: `ns0.li)ma.mike.`");
}

TEST(dns_read_zone_test, invalid_ttl_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. -999 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `-999`");
  expect_invalid_zone(
      "lima.mike. -1 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `-1`");
  expect_zone_eq(
      "lima.mike. 0 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      ".mike.lima 0 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5");
  expect_zone_eq(
      "lima.mike. 1 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      ".mike.lima 1 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5");
  expect_zone_eq(
      "lima.mike. 4294967295 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      ".mike.lima 4294967295 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5");
  expect_invalid_zone(
      "lima.mike. 4294967296 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `4294967296`");
  expect_invalid_zone(
      "lima.mike. 9999999999 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `9999999999`");
}

TEST(dns_read_zone_test, invalid_record_class_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 60 CH SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: unsupported resource record class: `CH`");
  expect_invalid_zone(
      "lima.mike. 60 HS SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: unsupported resource record class: `HS`");
}

TEST(dns_read_zone_test, unexpected_record_type_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 3600 IN soa ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: unsupported resource record type: `soa`");
  expect_invalid_zone(
      "lima.mike. 3600 IN sOa ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: unsupported resource record type: `sOa`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "lima.mike. 3600 IN TXT \"alpha=bravo charlie=delta echo=foxtrot\"",
      "line 2: unsupported resource record type: `TXT`");
}

TEST(dns_read_zone_test, soa_invalid_serial_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. -999 2 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `-999`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. -1 2 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `-1`");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 0 2 3 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 0 2 3 4 5");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 5");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 4294967295 2 3 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 4294967295 2 3 4 5");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 4294967296 2 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `4294967296`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 9999999999 2 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `9999999999`");
}

TEST(dns_read_zone_test, soa_invalid_refresh_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 -999 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `-999`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 -1 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `-1`");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 0 3 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 0 3 4 5");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 1 3 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 1 3 4 5");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 4294967295 3 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 4294967295 3 4 5");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 4294967296 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `4294967296`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 9999999999 3 4 5",
      "line 1: invalid unsigned 32 bit integer: `9999999999`");
}

TEST(dns_read_zone_test, soa_invalid_retry_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 -999 4 5",
      "line 1: invalid unsigned 32 bit integer: `-999`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 -1 4 5",
      "line 1: invalid unsigned 32 bit integer: `-1`");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 0 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 0 4 5");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 1 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 1 4 5");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 4294967295 4 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 4294967295 4 5");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 4294967296 4 5",
      "line 1: invalid unsigned 32 bit integer: `4294967296`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 9999999999 4 5",
      "line 1: invalid unsigned 32 bit integer: `9999999999`");
}

TEST(dns_read_zone_test, soa_invalid_expire_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 -999 5",
      "line 1: invalid unsigned 32 bit integer: `-999`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 -1 5",
      "line 1: invalid unsigned 32 bit integer: `-1`");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 0 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 0 5");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 1 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 1 5");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4294967295 5",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4294967295 5");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4294967296 5",
      "line 1: invalid unsigned 32 bit integer: `4294967296`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 9999999999 5",
      "line 1: invalid unsigned 32 bit integer: `9999999999`");
}

TEST(dns_read_zone_test, soa_invalid_minimum_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 -999",
      "line 1: invalid unsigned 32 bit integer: `-999`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 -1",
      "line 1: invalid unsigned 32 bit integer: `-1`");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 0",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 0");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 1",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 1");
  expect_zone_eq(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 4294967295",
      ".mike.lima 9 IN SOA .mike.lima.ns0 .mike.lima.admin 1 2 3 4 4294967295");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 4294967296",
      "line 1: invalid unsigned 32 bit integer: `4294967296`");
  expect_invalid_zone(
      "lima.mike. 9 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 9999999999",
      "line 1: invalid unsigned 32 bit integer: `9999999999`");
}

TEST(dns_read_zone_test, invalid_ip_address_is_not_ok) {
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN A 1.2.3.444",
      "line 2: invalid IPv4 address: `1.2.3.444`");
  expect_invalid_zone(
      "lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5\n"
      "ns0.lima.mike. 600 IN AAAA 1:2:3:4:c:d:e:g",
      "line 2: invalid IPv6 address: `1:2:3:4:c:d:e:g`");
}

TEST(dns_read_zone_test, no_records_is_not_ok) {
  expect_invalid_zone("", "the zone has no resource records");
}

TEST(dns_read_zone_test, leading_space_or_tab_is_not_ok) {
  expect_invalid_zone(
      " lima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: the line starts with a space");
  expect_invalid_zone(
      "\tlima.mike. 3600 IN SOA ns0.lima.mike. admin.lima.mike. 1 2 3 4 5",
      "line 1: invalid domain name: `\tlima.mike.`");
}

TEST(dns_read_zone_test, valid_zone_is_ok) {
  expect_zone_eq(
      "lima.mike. 111 IN SOA ns0.lima.mike. admin.lima.mike. 11 22 33 44 55\n"
      "lima.mike. 222 IN NS ns0.lima.mike.\n"
      "ns0.lima.mike. 444 IN CNAME alpha.lima.mike.\n"
      "alpha.lima.mike. 666 IN A 1.2.3.4\n"
      "alpha.lima.mike. 777 IN AAAA 1:2:3:4:c:d:e:f\n",
      ".mike.lima 111 IN SOA .mike.lima.ns0 .mike.lima.admin 11 22 33 44 55\n"
      ".mike.lima 222 IN NS .mike.lima.ns0\n"
      ".mike.lima.ns0 444 IN CNAME .mike.lima.alpha\n"
      ".mike.lima.alpha 666 IN A 1.2.3.4\n"
      ".mike.lima.alpha 777 IN AAAA 1:2:3:4:c:d:e:f");
  expect_zone_eq(
      "lima.mike. 111 IN SOA ns0.lima.mike. admin.lima.mike. 11 22 33 44 55\n"
      "lima.mike. 222 IN NS ns0.lima.mike.\n"
      "lima.mike. 333 IN NS ns1.lima.mike.\n"
      "ns0.lima.mike. 444 IN CNAME alpha.lima.mike.\n"
      "ns1.lima.mike. 555 IN CNAME bravo.lima.mike.\n"
      "alpha.lima.mike. 666 IN A 1.2.3.4\n"
      "alpha.lima.mike. 777 IN AAAA 1:2:3:4:c:d:e:f\n"
      "bravo.lima.mike. 888 IN A 5.6.7.8\n"
      "bravo.lima.mike. 999 IN AAAA 5:6:7:8:a:b:c:d",
      ".mike.lima 111 IN SOA .mike.lima.ns0 .mike.lima.admin 11 22 33 44 55\n"
      ".mike.lima 222 IN NS .mike.lima.ns0\n"
      ".mike.lima 333 IN NS .mike.lima.ns1\n"
      ".mike.lima.ns0 444 IN CNAME .mike.lima.alpha\n"
      ".mike.lima.ns1 555 IN CNAME .mike.lima.bravo\n"
      ".mike.lima.alpha 666 IN A 1.2.3.4\n"
      ".mike.lima.alpha 777 IN AAAA 1:2:3:4:c:d:e:f\n"
      ".mike.lima.bravo 888 IN A 5.6.7.8\n"
      ".mike.lima.bravo 999 IN AAAA 5:6:7:8:a:b:c:d");
}

TEST(dns_read_zone_test, comments_are_ok) {
  expect_zone_eq(
      ";\n; This is a sample domain zone.\n;\n"
      "lima.mike. 111 IN SOA ns0.lima.mike. admin.lima.mike. 11 22 33 44 55\n"
      "lima.mike. 222 IN NS ns0.lima.mike.; some comment\n"
      "ns0.lima.mike. 444 IN CNAME alpha.lima.mike.\n"
      "alpha.lima.mike. 666 IN A 1.2.3.4; another comment\n"
      "alpha.lima.mike. 777 IN AAAA 1:2:3:4:c:d:e:f\n",
      ".mike.lima 111 IN SOA .mike.lima.ns0 .mike.lima.admin 11 22 33 44 55\n"
      ".mike.lima 222 IN NS .mike.lima.ns0\n"
      ".mike.lima.ns0 444 IN CNAME .mike.lima.alpha\n"
      ".mike.lima.alpha 666 IN A 1.2.3.4\n"
      ".mike.lima.alpha 777 IN AAAA 1:2:3:4:c:d:e:f");
}

TEST(dns_read_zone_test, empty_lines_are_ok) {
  expect_zone_eq(
      "\nlima.mike. 111 IN SOA ns0.lima.mike. admin.lima.mike. 11 22 33 44 55\n"
      "\n\nlima.mike. 222 IN NS ns0.lima.mike.\n\n"
      "ns0.lima.mike. 444 IN CNAME alpha.lima.mike.\n"
      "alpha.lima.mike. 666 IN A 1.2.3.4\n"
      "alpha.lima.mike. 777 IN AAAA 1:2:3:4:c:d:e:f\n\n\n",
      ".mike.lima 111 IN SOA .mike.lima.ns0 .mike.lima.admin 11 22 33 44 55\n"
      ".mike.lima 222 IN NS .mike.lima.ns0\n"
      ".mike.lima.ns0 444 IN CNAME .mike.lima.alpha\n"
      ".mike.lima.alpha 666 IN A 1.2.3.4\n"
      ".mike.lima.alpha 777 IN AAAA 1:2:3:4:c:d:e:f");
}

TEST(dns_read_zone_test, whitespace_only_lines_are_ok) {
  expect_zone_eq(
      "    \n"
      "lima.mike. 111 IN SOA ns0.lima.mike. admin.lima.mike. 11 22 33 44 55\n"
      "  \n \n            \n"
      "lima.mike. 222 IN NS ns0.lima.mike.\n  \n"
      "ns0.lima.mike. 444 IN CNAME alpha.lima.mike.\n"
      "alpha.lima.mike. 666 IN A 1.2.3.4\n  \n"
      "alpha.lima.mike. 777 IN AAAA 1:2:3:4:c:d:e:f\n",
      ".mike.lima 111 IN SOA .mike.lima.ns0 .mike.lima.admin 11 22 33 44 55\n"
      ".mike.lima 222 IN NS .mike.lima.ns0\n"
      ".mike.lima.ns0 444 IN CNAME .mike.lima.alpha\n"
      ".mike.lima.alpha 666 IN A 1.2.3.4\n"
      ".mike.lima.alpha 777 IN AAAA 1:2:3:4:c:d:e:f");
}

TEST(dns_read_zone_test, heavily_annotated_zone_is_ok) {
  expect_zone_eq(
      ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n"
      "; This is a testing domain zone.\n"
      ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;\n\n\n   \n"
      "lima.mike. 111 IN SOA ns0.lima.mike. admin.lima.mike. 11 22 33 44 55\n"
      "\n\n                       \n    ; more comments here;;  \n"
      "lima.mike. 222 IN NS ns0.lima.mike.   ; some comment\n"
      "ns0.lima.mike. 444 IN CNAME alpha.lima.mike.\n"
      "alpha.lima.mike. 666 IN A 1.2.3.4     ; another comment\n"
      "\n\n  ;; even more comment here   \n           \n    \n"
      "alpha.lima.mike. 777 IN AAAA 1:2:3:4:c:d:e:f\n",
      ".mike.lima 111 IN SOA .mike.lima.ns0 .mike.lima.admin 11 22 33 44 55\n"
      ".mike.lima 222 IN NS .mike.lima.ns0\n"
      ".mike.lima.ns0 444 IN CNAME .mike.lima.alpha\n"
      ".mike.lima.alpha 666 IN A 1.2.3.4\n"
      ".mike.lima.alpha 777 IN AAAA 1:2:3:4:c:d:e:f");
}
