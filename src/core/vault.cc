#include "core/vault.h"
#include <filesystem>
#include <fstream>

namespace guicrypt {

namespace {

std::string strip_trailing_slash(const std::string& p) {
  if (p.size() > 1 && p.back() == '/') return p.substr(0, p.size() - 1);
  return p;
}

} // anon ns

bool Vault::valid() const {
  std::error_code ec;
  return std::filesystem::exists(path, ec) && !name.empty();
}

bool Vault::mounted() const {
  if (mount_point.empty()) return false;
  auto needle = strip_trailing_slash(mount_point);
  std::ifstream mounts("/proc/mounts");
  std::string line;
  while (std::getline(mounts, line)) {
    auto first_space = line.find(' ');
    if (first_space == std::string::npos) continue;
    auto second_space = line.find(' ', first_space + 1);
    if (second_space == std::string::npos) continue;
    auto mp = strip_trailing_slash(line.substr(first_space + 1, second_space - first_space - 1));
    if (mp == needle) return true;
  }
  return false;
}

bool Vault::stale_mount_point() const {
  if (mount_point.empty() || mounted()) return false;
  std::error_code ec;
  return std::filesystem::exists(mount_point, ec);
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
