#pragma once

#include <string>

namespace guicrypt {

class Gocryptfs {
public:
  static bool check_installed();
  static bool install();
  static bool mount(const std::string& path, const std::string& mount_point, const std::string& password);
  static bool unmount(const std::string& mount_point);
  static bool create(const std::string& path, const std::string& password);
  static bool remove(const std::string& path);
  static bool change_password(const std::string& path, const std::string& old_pass, const std::string& new_pass);
};

} // namespace guicrypt
