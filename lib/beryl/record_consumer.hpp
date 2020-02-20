#pragma once

#include <memory>

namespace beryl {
class domain_name;
class resource_record;
class record_consumer {
public:
  virtual ~record_consumer() = default;
  virtual void consume_zone_begin() = 0;
  virtual void consume_zone_end() = 0;
  virtual void
  consume(domain_name&& name, std::unique_ptr<resource_record> rr) = 0;
};
}  // namespace beryl
