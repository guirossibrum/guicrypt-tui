#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace guicrypt {

struct Vault {
  int id = 0;
  std::string name;
  std::string path;
  std::string mount_point;

  bool valid() const;
  bool mounted() const;
};

void to_json(nlohmann::json& j, const Vault& v);
void from_json(const nlohmann::json& j, Vault& v);

} // namespace guicrypt
