#include "core/vault_store.h"
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace guicrypt {

VaultStore::VaultStore() {
  load();
}

std::string VaultStore::path() const {
  const char* home = std::getenv("HOME");
  if (!home) return "vaults.json";
  return std::string(home) + "/.config/guicrypt-tui/vaults.json";
}

const std::vector<Vault>& VaultStore::all() const {
  return vaults_;
}

std::optional<Vault> VaultStore::find(int id) const {
  for (const auto& v : vaults_) {
    if (v.id == id) return v;
  }
  return std::nullopt;
}

int VaultStore::next_id() const {
  int max_id = 0;
  for (const auto& v : vaults_) {
    if (v.id > max_id) max_id = v.id;
  }
  return max_id + 1;
}

void VaultStore::add(const Vault& vault) {
  vaults_.push_back(vault);
  save();
}

void VaultStore::remove(int id) {
  vaults_.erase(
    std::remove_if(vaults_.begin(), vaults_.end(),
      [id](const Vault& v) { return v.id == id; }),
    vaults_.end()
  );
  save();
}

void VaultStore::remove_by_path(const std::string& path) {
  vaults_.erase(
    std::remove_if(vaults_.begin(), vaults_.end(),
      [&path](const Vault& v) { return v.path == path; }),
    vaults_.end()
  );
  save();
}

void VaultStore::load() {
  std::ifstream f(path());
  if (!f.is_open()) return;
  try {
    nlohmann::json j;
    f >> j;
    vaults_ = j.get<std::vector<Vault>>();
  } catch (...) {
    vaults_.clear();
  }
}

void VaultStore::save() const {
  auto p = path();
  auto dir = p.substr(0, p.find_last_of('/'));
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  std::ofstream f(p);
  if (!f.is_open()) return;
  nlohmann::json j = vaults_;
  f << j.dump(2) << std::endl;
}

} // namespace guicrypt
