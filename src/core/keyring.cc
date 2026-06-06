#include "core/keyring.h"
#include <cstdlib>
#include <string>
#include "util/exec.h"

namespace guicrypt {

bool Keyring::store(const std::string& name, const std::string& password) {
  std::string cmd = "secret-tool store --label='guicrypt-tui' vault " + name + " 2>/dev/null <<< \"" + password + "\"";
  return system(cmd.c_str()) == 0;
}

std::string Keyring::retrieve(const std::string& name) {
  std::string cmd = "secret-tool lookup vault " + name + " 2>/dev/null";
  std::string result = exec(cmd);
  if (!result.empty() && result.back() == '\n') result.pop_back();
  return result;
}

void Keyring::remove(const std::string& name) {
  std::string cmd = "secret-tool clear vault " + name + " 2>/dev/null";
  system(cmd.c_str());
}

} // namespace guicrypt
