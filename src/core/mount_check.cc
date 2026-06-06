#include "core/mount_check.h"
#include <fstream>

namespace guicrypt {

bool MountCheck::is_mounted(const std::string& mount_point) {
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

} // namespace guicrypt
