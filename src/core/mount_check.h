#pragma once

#include <string>

namespace guicrypt {

class MountCheck {
public:
  static bool is_mounted(const std::string& mount_point);
};

} // namespace guicrypt
