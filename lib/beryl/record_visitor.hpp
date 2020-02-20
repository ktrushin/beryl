#pragma once

#include <cstdint>

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>

#include "beryl/record_class.hpp"
#include "beryl/record_type.hpp"
#include "beryl/string.hpp"

namespace beryl {
class domain_name;
class record_visitor {
public:
  virtual ~record_visitor() = default;
  virtual void visit_record_begin() = 0;
  virtual void visit_record_end() = 0;
  virtual void visit(record_class) = 0;
  virtual void visit(record_type) = 0;
  virtual void visit(std::uint32_t) = 0;
  virtual void visit(const domain_name&) = 0;
  virtual void visit(boost::asio::ip::address_v4) = 0;
  virtual void visit(const boost::asio::ip::address_v6&) = 0;
};
}  // namespace beryl
