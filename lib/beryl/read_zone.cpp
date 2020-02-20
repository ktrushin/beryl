#include "beryl/read_zone.hpp"

#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <boost/tokenizer.hpp>

#include "beryl/domain_name.hpp"
#include "beryl/record_class.hpp"
#include "beryl/record_consumer.hpp"
#include "beryl/record_type.hpp"
#include "beryl/resource_record.hpp"

namespace beryl {
namespace {
std::string normalize_line(std::string&& line) {
  // remove comments
  if (std::size_t pos = line.find_first_of(';'); pos != std::string::npos) {
    line.erase(pos);
  }
  // remove trailing whitespaces
  std::size_t last_not_space = line.find_last_not_of(' ');
  line.erase(last_not_space == std::string::npos ? 0 : last_not_space + 1);

  return std::move(line);
}

void verify_record_class(const std::string& rc) {
  if (to_record_class(rc) == record_class::in) {
    return;
  }
}

void verify_single_soa_record(const domain_name& name, record_type rt,
                              std::optional<domain_name>& zone_domain) {
  if (zone_domain) {
    if (rt == record_type::soa) {
      throw std::runtime_error("second SOA record in the zone");
    }
  } else {
    if (rt == record_type::soa) {
      zone_domain = name;
      return;
    }
    throw std::runtime_error(
        "a zone must start with a SOA record but start with a " +
        to_string(rt) + " record");
  }
}

// Number of data tokens for resource record of different types.
// Data tokens do not include common part for all resource records, i.e.
// name, TTL, class (`IN`), type
std::size_t data_token_count(record_type rt) {
  // clang-format off
  switch (rt) {
    // IPv4 address
    case record_type::a: return 1;  // NOLINT(readability-magic-numbers)
    // nameserver name
    case record_type::ns: return 1;  // NOLINT(readability-magic-numbers)
    // canonical name
    case record_type::cname: return 1;  // NOLINT(readability-magic-numbers)
    // primary nameserver name, domain admin mailbox, serial, refresh, retry,
    // expire, minimum
    case record_type::soa: return 7;  // NOLINT(readability-magic-numbers)
    // IPv6 address
    case record_type::aaaa: return 1;  // NOLINT(readability-magic-numbers)
  }
  // clang-format off
  assert(false && "unexpected record type");
}

inline std::size_t max_data_token_count() {
  return data_token_count(record_type::soa);
}

void verify_data_token_count(const std::vector<std::string>& data_tokens) {
  if (data_tokens.size() > max_data_token_count()) {
    throw std::runtime_error("too many tokens in the resource record");
  }
}

class invalid_uint32 : public std::runtime_error {
public:
  invalid_uint32(const std::string& uint) : std::runtime_error(gen_msg(uint)) {}

private:
  static std::string gen_msg(const std::string& uint32_str) {
    std::ostringstream oss;
    oss << "invalid unsigned 32 bit integer: `" << uint32_str << "`";
    return oss.str();
  }
};

std::uint32_t str_to_uint32(const std::string& str) {
  try {
    static constexpr int base = 10;
    auto ll = stoll(str, nullptr, base);
    auto uint32_max =
        static_cast<decltype(ll)>(std::numeric_limits<std::uint32_t>::max());
    if (ll < 0 || ll > uint32_max) {
      throw invalid_uint32(str);
    }
    return static_cast<std::uint32_t>(ll);
  } catch (const std::invalid_argument&) {
    throw invalid_uint32(str);
  } catch (const std::out_of_range&) { throw invalid_uint32(str); }
}

template <record_type RecordType>
std::unique_ptr<resource_record>
make_record(std::uint32_t ttl, const std::vector<std::string>& data_tokens) {
  if (std::size_t got = data_tokens.size(),
      expected = data_token_count(RecordType);
      got != expected) {
    std::ostringstream msg;
    msg << "too " << (got < expected ? "few" : "many")
        << " tokens in the resource record";
    throw std::runtime_error(msg.str());
  }
  auto d = [&data_tokens](std::size_t index) {
    return domain_name(data_tokens[index]);
  };
  auto ui32 = [&data_tokens](std::size_t index) {
    return str_to_uint32(data_tokens[index]);
  };

  // In the next block, the following checks fail:
  // -- readability-braces-around-statements;
  // -- readability-misleading-indentation;
  // -- readability-magic-numbers.
  // The first two are triggered by `if constexpr` and `else if constexpr`,
  // the last one -- by calls to `ui32` with numeric indices.
  if constexpr (RecordType == record_type::soa) {  // NOLINT
    // NOLINTNEXTLINE
    return std::make_unique<soa_record>(ttl, d(0), d(1), ui32(2), ui32(3),
                                        // NOLINTNEXTLINE
                                        ui32(4), ui32(5), ui32(6));
  } else if constexpr (RecordType == record_type::ns) {  // NOLINT
    return std::make_unique<ns_record>(ttl, d(0));
  } else if constexpr (RecordType == record_type::cname) {  // NOLINT
    return std::make_unique<cname_record>(ttl, d(0));
  } else if constexpr (RecordType == record_type::a) {  // NOLINT
    return std::make_unique<a_record>(ttl, data_tokens[0]);
  } else if constexpr (RecordType == record_type::aaaa) {  // NOLINT
    return std::make_unique<aaaa_record>(ttl, data_tokens[0]);
  }
  assert(false && "unexpected record type");
  (void)d;     // suppress `variable ‘d’ set but not used` gcc error
  (void)ui32;  // suppress `variable ‘ui32’ set but not used` gcc error
}

std::unique_ptr<resource_record>
make_record(record_type rt, std::uint32_t ttl,
            const std::vector<std::string>& data_tokens) {
  switch (rt) {
    case record_type::a: return make_record<record_type::a>(ttl, data_tokens);
    case record_type::ns: return make_record<record_type::ns>(ttl, data_tokens);
    case record_type::cname:
      return make_record<record_type::cname>(ttl, data_tokens);
    case record_type::soa:
      return make_record<record_type::soa>(ttl, data_tokens);
    case record_type::aaaa:
      return make_record<record_type::aaaa>(ttl, data_tokens);
  }
  assert(false && "unexpected record type");
}
}  // namespace

void read_zone(std::istream& is, record_consumer& consumer) {
  std::vector<std::string> data_tokens;
  data_tokens.reserve(max_data_token_count());

  boost::char_separator<char> sep{" ", "", boost::drop_empty_tokens};
  using tokenizer = boost::tokenizer<boost::char_separator<char>>;

  consumer.consume_zone_begin();
  std::uint32_t line_number = 0;
  std::optional<domain_name> zone_domain;
  try {
    for (std::string line; std::getline(is, line);) {
      ++line_number;
      line = normalize_line(std::move(line));
      if (line.empty()) {
        continue;
      }
      if (str_starts_with(line, ' ')) {
        throw std::runtime_error("the line starts with a space");
      }

      tokenizer tokens{line, sep};
      tokenizer::iterator token = tokens.begin();
      // clang-format off
      auto next_token = [&token, end = tokens.end()]() {
        return token != end
          ? *token++
          : throw std::runtime_error("too few tokens in the resource record");
      };
      // clang-format on
      domain_name name(next_token());
      auto ttl = str_to_uint32(next_token());
      verify_record_class(next_token());
      record_type rt = to_record_type(next_token());
      verify_single_soa_record(name, rt, zone_domain);

      data_tokens.clear();
      for (; token != tokens.end(); ++token) {
        data_tokens.push_back(*token);
        verify_data_token_count(data_tokens);
      }

      consumer.consume(std::move(name), make_record(rt, ttl, data_tokens));
    }
  } catch (const std::runtime_error& e) {
    std::ostringstream msg;
    msg << "line " << line_number << ": " << e.what();
    throw std::runtime_error(msg.str());
  }
  if (!zone_domain) {
    throw std::runtime_error("the zone has no resource records");
  }
  consumer.consume_zone_end();
}
}  // namespace beryl
