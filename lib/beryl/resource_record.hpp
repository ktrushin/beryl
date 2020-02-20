#pragma once

#include <cstdint>

#include <memory>
#include <ostream>
#include <type_traits>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>

#include "beryl/chrono.hpp"
#include "beryl/domain_name.hpp"
#include "beryl/record_class.hpp"
#include "beryl/record_type.hpp"
#include "beryl/record_visitor.hpp"
#include "beryl/string.hpp"

namespace beryl {
class resource_record {
public:
  virtual ~resource_record() = default;

  [[nodiscard]] virtual record_type type() const noexcept = 0;
  void accept(record_visitor& v) const {
    v.visit_record_begin();
    v.visit(ttl());
    v.visit(record_class::in);
    v.visit(type());
    accept_specific(v);
    v.visit_record_end();
  }

  // Ideally, these two method shouldn't exist but there is no any other
  // practical means of storing resource records of different types in one
  // container and distinguishing between them.
  template <typename T>
  T* cast() noexcept {
    return static_cast<T*>(this);
  }
  template <typename T>
  const T* cast() const noexcept {
    return static_cast<const T*>(this);
  }

  [[nodiscard]] std::uint32_t
  ttl(const chrono::time_point& now = chrono::now()) const noexcept {
    if ((_t & discriminator) != 0) {
      chrono::time_point expiration(chrono::seconds(_t & ~discriminator));
      return now < expiration
                 ? static_cast<std::uint32_t>((expiration - now).count())
                 : 0;
    }
    return static_cast<std::uint32_t>(_t);
  }

protected:
  explicit resource_record(std::uint32_t ttl) noexcept : _t(ttl) {}
  explicit resource_record(const chrono::time_point& expiration) noexcept {
    auto seconds = expiration.time_since_epoch().count();
    assert(seconds >= 0 && "expiration time can't be less than epoch");
    _t = discriminator +
         std::min(static_cast<std::uint64_t>(seconds), discriminator - 1);
  }
  virtual void accept_specific(record_visitor& v) const = 0;

private:
  // If the most significant bit is set, then the remaining 63 bits
  // represent resource record expiration time as a count of seconds
  // from the epoch.
  // Otherwise, the 32 least significant bits represent resource record TTL.
  std::uint64_t _t;
  // clang-format off
  static constexpr std::uint64_t discriminator =
      static_cast<std::uint64_t>(1) << 63;
  // clang-format on
};

namespace _impl {
struct a_record_traits {
  using addr_type = boost::asio::ip::address_v4;
  static constexpr record_type type = record_type::a;
  static addr_type make_addr(typename addr_type::bytes_type bytes) {
    return boost::asio::ip::make_address_v4(bytes);
  }
  template <typename Arg>
  static addr_type make_addr(Arg&& arg) {
    try {
      return boost::asio::ip::make_address_v4(std::forward<Arg>(arg));
    } catch (const std::runtime_error&) {
      std::ostringstream msg;
      msg << "invalid IPv4 address: `" << arg << "`";
      throw std::runtime_error(msg.str());
    }
  }
};

struct aaaa_record_traits {
  using addr_type = boost::asio::ip::address_v6;
  static constexpr record_type type = record_type::aaaa;
  static addr_type make_addr(typename addr_type::bytes_type bytes) {
    return boost::asio::ip::make_address_v6(bytes);
  }
  template <typename Arg>
  static addr_type make_addr(Arg&& arg) {
    try {
      return boost::asio::ip::make_address_v6(std::forward<Arg>(arg));
    } catch (const std::runtime_error&) {
      std::ostringstream msg;
      msg << "invalid IPv6 address: `" << arg << "`";
      throw std::runtime_error(msg.str());
    }
  }
};

template <typename RecordTraits>
class addr_record : public resource_record {
public:
  template <typename T0, typename T1>
  addr_record(T0&& t, T1&& addr)
      : resource_record(std::forward<T0>(t)),
        address_bytes(
            RecordTraits::make_addr(std::forward<T1>(addr)).to_bytes()) {}
  [[nodiscard]] record_type type() const noexcept final {
    return RecordTraits::type;
  }
  void accept_specific(record_visitor& v) const final { v.visit(address()); }
  typename RecordTraits::addr_type address() const noexcept {
    return typename RecordTraits::addr_type(address_bytes);
  }

private:
  // The class stores bytes of the address rather then address itself because
  // `boost::asio::ip::address_v6` keeps scope id besides bytes,
  // thus waisting additional 8 bytes per record.
  typename RecordTraits::addr_type::bytes_type address_bytes;
};

template <record_type Type>
class domain_record : public resource_record {
public:
  template <typename T0, typename T1>
  domain_record(T0&& t, T1&& domain_name)
      : resource_record(std::forward<T0>(t)),
        name(std::forward<T1>(domain_name)) {}
  [[nodiscard]] record_type type() const noexcept final { return Type; }
  void accept_specific(record_visitor& v) const final { v.visit(name); }

  domain_name name;  // NOLINT(misc-non-private-member-variables-in-classes)
};
}  // namespace _impl

// clang-format off
using     a_record = _impl::addr_record<_impl::a_record_traits>;
using  aaaa_record = _impl::addr_record<_impl::aaaa_record_traits>;
using    ns_record = _impl::domain_record<record_type::ns>;
using cname_record = _impl::domain_record<record_type::cname>;
// clang-format on

class soa_record : public resource_record {
public:
  template <typename T0, typename T1, typename T2>
  soa_record(T0&& t, T1&& nameserver, T2&& mailbox, std::uint32_t serial,
             std::uint32_t refresh, std::uint32_t retry, std::uint32_t expire,
             std::uint32_t min_ttl)
      : resource_record(std::forward<T0>(t)),
        nameserver(std::forward<T1>(nameserver)),
        mailbox(std::forward<T2>(mailbox)),
        serial(serial),
        refresh(refresh),
        retry(retry),
        expire(expire),
        min_ttl(min_ttl) {}

  [[nodiscard]] record_type type() const noexcept final {
    return record_type::soa;
  }
  void accept_specific(record_visitor& v) const final {
    v.visit(nameserver);
    v.visit(mailbox);
    v.visit(serial);
    v.visit(refresh);
    v.visit(retry);
    v.visit(expire);
    v.visit(min_ttl);
  }

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  domain_name nameserver;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  domain_name mailbox;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::uint32_t serial;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::uint32_t refresh;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::uint32_t retry;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::uint32_t expire;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::uint32_t min_ttl;
};
}  // namespace beryl
