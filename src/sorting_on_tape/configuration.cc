#include "configuration.h"

#include <fstream>
#include <limits>

namespace sot {

Configuration::Configuration(const std::string &file_name) {
  ReadParamsFromFile(file_name);
}

std::uint64_t Configuration::GetProperty(const std::string &key, const std::uint64_t default_value)
    const {
  const auto value = params.find(key);
  if (value != params.end()) {
    return value->second;
  }
  return default_value;
}

void Configuration::ReadParamsFromFile(const std::string &file_name) {
  std::ifstream input(file_name);
  if (!input) {
    return;
  }
  std::string key;
  while (getline(input, key, '=')) {
    if (!key.starts_with("#")) {
      uint64_t value;
      input >> value;
      if (input.fail()) {
        input.clear();
      } else {
        params.emplace(key, value);
      }
    }
    input.ignore(std::numeric_limits<std::streamsize>::max(), input.widen('\n'));
  }
}

}  // namespace sot
