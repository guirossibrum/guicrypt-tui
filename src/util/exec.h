#pragma once

#include <string>
#include <array>
#include <memory>
#include <cstdio>

namespace guicrypt {

inline std::string exec(const std::string& cmd) {
  std::array<char, 1024> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) return "";
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }
  return result;
}

} // namespace guicrypt
