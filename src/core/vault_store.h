#pragma once

#include <vector>
#include <optional>
#include "core/vault.h"

namespace guicrypt {

class VaultStore {
public:
  VaultStore();

  const std::vector<Vault>& all() const;
  std::optional<Vault> find(int id) const;
  int next_id() const;
  void add(const Vault& vault);
  void remove(int id);
  void remove_by_path(const std::string& path);

private:
  std::string path() const;
  std::vector<Vault> vaults_;
  void load();
  void save() const;
};

} // namespace guicrypt
