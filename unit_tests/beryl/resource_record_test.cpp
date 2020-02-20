#include "beryl/resource_record.hpp"

#include <limits>
#include <string>

#include <gtest/gtest.h>

#include "beryl/chrono.hpp"

#include "unit_testing/expect_throw_msg_eq.hpp"

using beryl::a_record;
using beryl::aaaa_record;
using beryl::cname_record;
using beryl::ns_record;
using beryl::record_type;
using beryl::soa_record;

namespace {
template <typename T, typename... Args>
void test_ttl(const std::string& msg, Args&&... args) {
  SCOPED_TRACE(msg);
  {
    std::uint32_t ttl = 0;
    T rr(ttl, std::forward<Args>(args)...);
    EXPECT_EQ(rr.ttl(), ttl);
  }
  {
    std::uint32_t ttl = 1;
    T rr(ttl, std::forward<Args>(args)...);
    EXPECT_EQ(rr.ttl(), ttl);
  }
  {
    std::uint32_t ttl = 3600;
    T rr(ttl, std::forward<Args>(args)...);
    EXPECT_EQ(rr.ttl(), ttl);
  }
  {
    std::uint32_t ttl = std::numeric_limits<std::uint32_t>::max() - 1;
    T rr(ttl, std::forward<Args>(args)...);
    EXPECT_EQ(rr.ttl(), ttl);
  }
  {
    std::uint32_t ttl = std::numeric_limits<std::uint32_t>::max();
    T rr(ttl, std::forward<Args>(args)...);
    EXPECT_EQ(rr.ttl(), ttl);
  }
  {
    auto now = beryl::chrono::now();
    T rr(now + std::chrono::hours(1), std::forward<Args>(args)...);
    // clang-format off
    EXPECT_EQ(rr.ttl(now - std::chrono::seconds(   1)), 3601);
    EXPECT_EQ(rr.ttl(now                             ), 3600);
    EXPECT_EQ(rr.ttl(now + std::chrono::seconds(   1)), 3599);
    EXPECT_EQ(rr.ttl(now + std::chrono::seconds(3599)),    1);
    EXPECT_EQ(rr.ttl(now + std::chrono::seconds(3600)),    0);
    EXPECT_EQ(rr.ttl(now + std::chrono::seconds(3601)),    0);
    // clang-format on
  }
}

template <typename T>
void test_uint32(const std::string& msg, T&& gen) {
  SCOPED_TRACE(msg);
  std::uint32_t min = std::numeric_limits<std::uint32_t>::min();
  std::uint32_t max = std::numeric_limits<std::uint32_t>::max();
  EXPECT_EQ(gen(min), min);
  EXPECT_EQ(gen(min + 1), min + 1);
  EXPECT_EQ(gen(123456789), 123456789);
  EXPECT_EQ(gen(max - 1), max - 1);
  EXPECT_EQ(gen(max), max);
}

template <typename T>
void test_domain_name(const std::string& msg, T&& gen) {
  SCOPED_TRACE(msg);
  std::ostringstream s;
  EXPECT_EQ(to_string(gen("foo.bar.baz.")), ".baz.bar.foo");
  EXPECT_EQ(to_string(gen("alpha.bravo.charlie.")), ".charlie.bravo.alpha");
}
}  // namespace

TEST(resource_record, ttl) {
  test_ttl<a_record>("a_record", "127.0.0.1");
  test_ttl<aaaa_record>("aaaa_record", "::1");
  test_ttl<ns_record>("ns_record", "ns0.lima.mike.");
  test_ttl<cname_record>("cname_record", "kilo.lima.mike.");
  test_ttl<soa_record>("soa_record", "ns0.lima.mike.", "admin.lima.mike.", 1u,
                       2u, 3u, 4u, 5u);
}

TEST(dns_resource_record_test, record_type) {
  EXPECT_EQ(a_record(3600u, "127.0.0.1").type(), record_type::a);
  EXPECT_EQ(aaaa_record(3600u, "::1").type(), record_type::aaaa);
  EXPECT_EQ(ns_record(0u, "ns0.foo.").type(), record_type::ns);
  EXPECT_EQ(cname_record(0u, "bar.foo.").type(), record_type::cname);
  EXPECT_EQ(soa_record(0u, "ns0.foo.", "admin.foo.", 0u, 0u, 0u, 0u, 0u).type(),
            record_type::soa);
}

TEST(dns_a_resource_record_test, valid_address_is_ok) {
  EXPECT_EQ(a_record(0u, "127.0.0.1").address().to_string(), "127.0.0.1");
  EXPECT_EQ(a_record(0u, "0.0.0.0").address().to_string(), "0.0.0.0");
  EXPECT_EQ(a_record(0u, "1.2.3.4").address().to_string(), "1.2.3.4");
  EXPECT_EQ(a_record(0u, "4.3.2.1").address().to_string(), "4.3.2.1");
  EXPECT_EQ(a_record(0u, "255.255.255.255").address().to_string(),
            "255.255.255.255");
}

TEST(dns_a_resource_record_test, invalid_address_is_not_ok) {
  auto expect_invalid_address = [](const std::string& addr) {
    SCOPED_TRACE(addr);
    std::string msg = "invalid IPv4 address: `" + addr + "`";
    EXPECT_THROW_MSG_EQ(a_record(0u, addr), std::runtime_error, msg.c_str());
  };
  expect_invalid_address("1.2.3.256");
  expect_invalid_address("999.2.3.1");
  expect_invalid_address("::1");
  expect_invalid_address("2001:53:ba3e::30");
  expect_invalid_address("foo_bar");
}

TEST(dns_aaaa_resource_record_test, valid_address_is_ok) {
  auto expect_address_eq = [](const std::string& address) {
    SCOPED_TRACE(address);
    EXPECT_EQ(aaaa_record(0u, address).address().to_string(), address);
  };
  expect_address_eq("::");
  expect_address_eq("::1");
  expect_address_eq("1:2:3:4:5:6:7:8");
  expect_address_eq("8:7:6:5:4:3:2:1");
  expect_address_eq("a:b:c:d:e::f");
  expect_address_eq("f:e:d:c:b::a");
  expect_address_eq("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
}

TEST(dns_aaaa_resource_record_test, invalid_address_is_not_ok) {
  auto expect_invalid_address = [](const std::string& addr) {
    SCOPED_TRACE(addr);
    std::string msg = "invalid IPv6 address: `" + addr + "`";
    EXPECT_THROW_MSG_EQ(aaaa_record(0u, addr), std::runtime_error, msg.c_str());
  };
  expect_invalid_address(":::");
  expect_invalid_address("::g");
  expect_invalid_address("a:b:c:d:e::g");
  expect_invalid_address("g:e:d:c:b::a");
  expect_invalid_address("1.2.3.4");
  expect_invalid_address("4.3.2.1");
  expect_invalid_address("127.0.0.1");
  expect_invalid_address("foo_bar");
}

TEST(dns_ns_resource_record_test, nameserver) {
  test_domain_name("ns nameserver", [](const std::string& ns) {
    return ns_record(0u, ns).name;
  });
}

TEST(dns_cname_resource_record_test, cname) {
  test_domain_name("cname canonical name", [](const std::string& cname) {
    return cname_record(0u, cname).name;
  });
}

TEST(dns_soa_resource_record_test, nameserver) {
  test_domain_name("soa nameserver", [](const std::string& ns) {
    return soa_record(0u, ns, "ns0.foo.", 1u, 2u, 3u, 4u, 5u).nameserver;
  });
}

TEST(dns_soa_resource_record_test, mailbox) {
  test_domain_name("soa mailbox", [](const std::string& mailbox) {
    return soa_record(0u, "ns0.foo.", mailbox, 1u, 2u, 3u, 4u, 5u).mailbox;
  });
}

TEST(dns_soa_resource_record_test, serial) {
  test_uint32("soa serial", [](std::uint32_t serial) {
    return soa_record(0u, "ns0.foo.", "admin.foo.", serial, 2u, 3u, 4u, 5u)
        .serial;
  });
}

TEST(dns_soa_resource_record_test, refresh) {
  test_uint32("soa refresh", [](std::uint32_t refresh) {
    return soa_record(0u, "ns0.foo.", "ns0.foo.", 1u, refresh, 3u, 4u, 5u)
        .refresh;
  });
}

TEST(dns_soa_resource_record_test, retry) {
  test_uint32("soa retry", [](std::uint32_t retry) {
    return soa_record(0u, "ns0.foo.", "admin.foo.", 1u, 2u, retry, 4u, 5u).retry;
  });
}

TEST(dns_soa_resource_record_test, expire) {
  test_uint32("soa expire", [](std::uint32_t expire) {
    return soa_record(0u, "ns0.foo.", "admin.foo.", 1u, 2u, 3u, expire, 5u)
        .expire;
  });
}

TEST(dns_soa_resource_record_test, min_ttl) {
  test_uint32("soa min_ttl", [](std::uint32_t min_ttl) {
    return soa_record(0u, "ns0.foo.", "admin.foo.", 1u, 2u, 3u, 4u, min_ttl)
        .min_ttl;
  });
}

TEST(dns_resource_record_test, size) {
  EXPECT_EQ(sizeof(a_record), 24);
  EXPECT_EQ(sizeof(aaaa_record), 32);
  EXPECT_EQ(sizeof(ns_record), 40);
  EXPECT_EQ(sizeof(cname_record), 40);
  EXPECT_EQ(sizeof(soa_record), 88);
}
