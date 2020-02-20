#include <iostream>

#include <boost/program_options.hpp>

#include "common/version.hpp"

int main(int argc, const char* argv[]) {
  try {
    namespace po = boost::program_options;
    po::options_description opt_desc{"Options"};
    // clang-format off
    opt_desc.add_options()
      ("help,h", "Print the help message")
      ("version,v", "Print version");
    // clang-format on

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, opt_desc), vm);
    po::notify(vm);

    if (vm.find("help") != vm.end()) {
      constexpr const char* desc_msg =
          "Description:\n"
          "  A primitive DNS nameserver.\n\n"
          "Usage:\n"
          "  beryl [options]";
      constexpr const char* example_msg =
          "Examples:\n"
          "  prompt> beryl\n"
          "  No meaningfull functionality yet.\n";
      std::cout << desc_msg << "\n\n"
                << opt_desc << "\n"
                << example_msg << std::endl;
      return 0;
    }
    if (vm.find("version") != vm.end()) {
      std::cout << common::version << std::endl;
      return 0;
    }
  } catch (const boost::program_options::error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << "No meaningfull functionality yet." << std::endl;
  return 0;
}
