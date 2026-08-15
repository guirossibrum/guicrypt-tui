#include "core/gocryptfs.h"
#include <cstdlib>
#include <string>

namespace guicrypt {

bool Gocryptfs::check_installed() {
  return system("command -v gocryptfs >/dev/null 2>&1") == 0;
}

bool Gocryptfs::install() {
  if (system("command -v omarchy-pkg-add >/dev/null 2>&1") == 0) {
    system("omarchy-notification-send \"guicrypt-tui\" \"Installing gocryptfs...\" >/dev/null 2>&1");
    return system("omarchy-pkg-add gocryptfs >/dev/null 2>&1") == 0;
  }
  return system("sudo pacman -S --noconfirm gocryptfs >/dev/null 2>&1") == 0;
}

bool Gocryptfs::mount(const std::string& path, const std::string& mount_point, const std::string& password) {
  std::string cmd = "mkdir -p \"" + mount_point + "\" && echo \"" + password + "\" | gocryptfs \"" + path + "\" \"" + mount_point + "\" >/dev/null 2>&1";
  return system(cmd.c_str()) == 0;
}

bool Gocryptfs::unmount(const std::string& mount_point) {
  std::string cmd = "fusermount -uz \"" + mount_point + "\" 2>/dev/null";
  return system(cmd.c_str()) == 0;
}

bool Gocryptfs::create(const std::string& path, const std::string& password) {
  std::string cmd = "mkdir -p \"" + path + "\" && echo \"" + password + "\" | gocryptfs -init \"" + path + "\" >/dev/null 2>&1";
  return system(cmd.c_str()) == 0;
}

bool Gocryptfs::remove(const std::string& path) {
  std::string cmd = "rm -rf \"" + path + "\" 2>/dev/null";
  return system(cmd.c_str()) == 0;
}

bool Gocryptfs::change_password(const std::string& path, const std::string& old_pass, const std::string& new_pass) {
  std::string cmd = "printf '%s\\n%s\\n%s\\n' \"" + old_pass + "\" \"" + new_pass + "\" \"" + new_pass + "\" | gocryptfs -passwd \"" + path + "\" >/dev/null 2>&1";
  return system(cmd.c_str()) == 0;
}

} // namespace guicrypt
