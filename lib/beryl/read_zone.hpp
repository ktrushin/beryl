#pragma once

#include <istream>
#include <memory>

namespace beryl {
class record_consumer;
void read_zone(std::istream& is, record_consumer& consumer);
}  // namespace beryl
