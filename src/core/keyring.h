#pragma once

#include <string>

namespace guicrypt {

class Keyring {
public:
  static bool store(const std::string& name, const std::string& password);
  static std::string retrieve(const std::string& name);
  static void remove(const std::string& name);
};

} // namespace guicrypt
