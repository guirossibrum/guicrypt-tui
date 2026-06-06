#include "core/vault.h"
#include <fstream>

namespace guicrypt {

bool Vault::valid() const {
  std::error_code ec;
  return std::filesystem::exists(path, ec) && !name.empty();
}

bool Vault::mounted() const {
  if (mount_point.empty()) return false;
  std::ifstream mounts("/proc/mounts");
  std::string line;
  while (std::getline(mounts, line)) {
    if (line.find(mount_point) != std::string::npos) {
      return true;
    }
  }
  return false;
}

void to_json(nlohmann::json& j, const Vault& v) {
  j = nlohmann::json{
    {"id", v.id},
    {"name", v.name},
    {"path", v.path},
    {"mount_point", v.mount_point}
  };
}

void from_json(const nlohmann::json& j, Vault& v) {
  j.at("id").get_to(v.id);
  j.at("name").get_to(v.name);
  j.at("path").get_to(v.path);
  j.at("mount_point").get_to(v.mount_point);
}

} // namespace guicrypt
