#pragma once

#include <cstdint>

#include <chrono>

namespace beryl::chrono {
using seconds = std::chrono::duration<std::int64_t>;
using time_point = std::chrono::time_point<std::chrono::system_clock, seconds>;
inline time_point now() {
  return time_point(std::chrono::duration_cast<time_point::duration>(
      std::chrono::system_clock::now().time_since_epoch()));
}
}  // namespace beryl::chrono
