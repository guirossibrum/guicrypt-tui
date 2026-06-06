#include "ui/screen.h"
#include "core/vault_store.h"
#include "core/vault.h"
#include "core/gocryptfs.h"
#include "core/keyring.h"
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/terminal.hpp>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

namespace {

std::string resolve_path(const std::string& p) {
  if (p.empty()) return p;
  std::string r = p;
  // Expand ~/ to $HOME/
  if (r.size() > 0 && r[0] == '~') {
    auto home = std::getenv("HOME");
    if (home) r = home + r.substr(1);
  }
  return fs::absolute(fs::path(r)).lexically_normal().string();
}

std::string vault_size_info(const std::string& path) {
  std::error_code ec;
  if (!fs::exists(path, ec)) return {};
  uintmax_t files = 0;
  uintmax_t size = 0;
  for (auto& entry : fs::recursive_directory_iterator(path, ec)) {
    if (entry.is_regular_file(ec)) {
      ++files;
      size += entry.file_size(ec);
    }
  }
  if (ec.value()) return {};
  std::string out = std::to_string(files) + (files == 1 ? " file" : " files");
  out += ", ";
  auto s = size;
  if (s < 1024)
    out += std::to_string(s) + " B";
  else if (s < 1024 * 1024)
    out += std::to_string(s / 1024) + " KB";
  else if (s < 1024 * 1024 * 1024)
    out += std::to_string(s / (1024 * 1024)) + " MB";
  else
    out += std::to_string(s / (1024 * 1024 * 1024)) + " GB";
  return out;
}

} // anon ns

namespace guicrypt {

using namespace ftxui;

Element VaultList(const std::vector<Vault>& vaults, int selected) {
  Elements items;
  for (size_t i = 0; i < vaults.size(); ++i) {
    auto& v = vaults[i];
    std::string icon = v.mounted() ? "●" : "○";
    std::string label = " " + icon + " " + v.name;
    auto el = text(label);
    if ((int)i == selected) el = el | inverted;
    if (v.mounted()) el = el | bold;
    items.push_back(el);
  }
  return vbox(std::move(items)) | flex;
}

Element DetailPanel(const Vault* vault) {
  if (!vault) return text("No vault selected") | dim | center;
  auto status_el = vault->mounted()
    ? text("● Mounted") | color(Color::Green)
    : text("○ Unmounted") | color(Color::GrayDark);
  return vbox({
    hbox({text("Name:  "), text(vault->name)}),
    hbox({text("Directory:  "), text(vault->path) | dim}),
    hbox({text("Mount: "), text(vault->mount_point) | dim}),
    separator(),
    hbox({text("Status:"), separator(), status_el}),
    separator(),
    text("  [m] Mount"),
    text("  [u] Unmount"),
    text("  [v] Vault Directory"),
    vault->mounted() ? text("  [o] Open Vault") : text(""),
    text(""),
    text("  [c] Change Password"),
  }) | flex;
}

void Screen::run() {
  auto screen = ScreenInteractive::Fullscreen();
  VaultStore store;

  struct State {
    int selected = 0;
    std::string status = "Ready";
    enum class Dialog { None, MountPassword, AddVault, NewVault, RemoveVault, ChangePass };
    Dialog dialog = Dialog::None;
    bool show_mount = false;
    bool show_add = false;
    bool show_new = false;
    bool show_remove = false;
    bool show_change_pass = false;
    int remove_choice = 0;
    bool remove_confirm = false;
    std::string remove_name;
    std::string remove_path;
    std::string remove_mount;

    std::string pass;
    std::string add_path;
    std::string add_mount;
    std::string new_path;
    std::string new_name;
    std::string new_mount;
    std::string new_pass;
    std::string new_confirm;
    std::string change_cur_pass;
    std::string change_new_pass;
    std::string change_confirm;
    std::string error;
  };
  auto S = std::make_shared<State>();

  // ── Input components ──────────────────────────────────────
  InputOption pass_opt;
  pass_opt.password = true;
  auto pass_input = Input(&S->pass, "password", pass_opt);

  auto add_path_input = Input(&S->add_path, "~/directory/vault");
  auto add_mount_input = Input(&S->add_mount, "/tmp/mount/point");

  auto new_name_input = Input(&S->new_name, "my-vault");
  auto new_path_input = Input(&S->new_path, "~/Documents/");
  auto new_mount_input = Input(&S->new_mount, "/tmp/my-vault");
  InputOption np_opt;
  np_opt.password = true;
  auto new_pass_input = Input(&S->new_pass, "", np_opt);
  auto new_confirm_input = Input(&S->new_confirm, "", np_opt);

  InputOption cp_opt;
  cp_opt.password = true;
  auto change_cur_input = Input(&S->change_cur_pass, "current password", cp_opt);
  auto change_new_input = Input(&S->change_new_pass, "new password", cp_opt);
  auto change_confirm_input = Input(&S->change_confirm, "confirm new password", cp_opt);

  // ── Main renderer ─────────────────────────────────────────
  auto main_renderer = Renderer([&, S](bool) {
    const auto& vaults = store.all();
    const Vault* selected = nullptr;
    if (!vaults.empty() && S->selected >= 0 && S->selected < (int)vaults.size())
      selected = &vaults[S->selected];
    auto left = window(text(" Vaults (" + std::to_string(vaults.size()) + ") "),
                       VaultList(vaults, S->selected));
    auto right = window(text(" Selected Vault "), DetailPanel(selected));
    auto status_status = text(" " + S->status) | bgcolor(Color::DarkBlue) | color(Color::White);
    auto status_hints = text(" [a]Add  [n]New  [r]Remove  [q]Quit ") |
                        bgcolor(Color::DarkBlue) | color(Color::White);
    auto dim = Terminal::Size();
    int left_w = std::max(20, dim.dimx / 3);
    return vbox({
             text(" guicrypt-tui ") | bold,
             separator(),
             hbox({left | size(WIDTH, EQUAL, left_w), separator(), right | flex}) | flex,
             separator(),
             hbox({status_status | flex, status_hints}),
           }) | border;
  });

  // ── Mount Password Dialog ─────────────────────────────────
  auto mount_container = Container::Vertical({pass_input});
  auto mount_renderer = Renderer(mount_container, [&, S] {
    return vbox({
      text(" Enter password ") | bold | center,
      separator(),
      text(" " + S->error),
      pass_input->Render() | flex,
      separator(),
      text(" [Enter] confirm  [Esc] cancel"),
    }) | border | center;
  });
  auto mount_component = mount_renderer | CatchEvent([&, S](Event event) {
    if (S->dialog != State::Dialog::MountPassword) return false;
    if (event == Event::Escape) {
      S->dialog = State::Dialog::None;
      S->show_mount = false;
      S->pass.clear();
      S->error.clear();
      return true;
    }
    if (event == Event::Return) {
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        auto& v = vaults[S->selected];
        if (Gocryptfs::mount(v.path, v.mount_point, S->pass)) {
          Keyring::store(v.name, S->pass);
          S->status = "Mounted " + v.name;
        } else {
          S->status = "Failed to mount " + v.name + " — check password";
        }
      }
      S->dialog = State::Dialog::None;
      S->show_mount = false;
      S->pass.clear();
      S->error.clear();
      return true;
    }
    return false;
  });

  // ── Add Vault Dialog ──────────────────────────────────────
  auto add_container = Container::Vertical({add_path_input, add_mount_input});
  auto add_renderer = Renderer(add_container, [&, S] {
    return vbox({
      text(" Add Existing Vault ") | bold | center,
      separator(),
      text(" Vault directory:"),
      add_path_input->Render(),
      text(" Mount point:"),
      add_mount_input->Render(),
      text(" " + S->error),
      separator(),
      text(" [Enter] confirm  [Esc] cancel"),
    }) | border | center;
  });
  auto add_component = add_renderer | CatchEvent([&, S](Event event) {
    if (S->dialog != State::Dialog::AddVault) return false;
    if (event == Event::Escape) {
      S->dialog = State::Dialog::None;
      S->show_add = false;
      S->add_path.clear();
      S->add_mount.clear();
      S->error.clear();
      return true;
    }
    if (event == Event::Return) {
      if (S->add_path.empty() || S->add_mount.empty()) {
        S->error = "Both fields are required.";
        return true;
      }
      auto resolved = resolve_path(S->add_path);
      if (!fs::exists(resolved)) {
        S->error = "Directory does not exist: " + resolved;
        return true;
      }
      Vault v;
      v.id = store.next_id();
      v.name = resolved.substr(resolved.find_last_of('/') + 1);
      v.path = resolved;
      v.mount_point = resolve_path(S->add_mount);
      store.add(v);
      S->status = "Added vault " + v.name;
      S->add_path.clear();
      S->add_mount.clear();
      S->error.clear();
      S->dialog = State::Dialog::None;
      S->show_add = false;
      return true;
    }
    return false;
  });

  // ── New Vault Dialog ──────────────────────────────────────
  auto new_container = Container::Vertical({
    new_name_input, new_path_input, new_mount_input,
    new_pass_input, new_confirm_input,
  });
  auto new_renderer = Renderer(new_container, [&, S] {
    return vbox({
      text(" Create New Vault ") | bold | center,
      separator(),
      text(" Vault name:"),
      new_name_input->Render(),
      text(" Vault directory:"),
      new_path_input->Render(),
      text(" Mount point:"),
      new_mount_input->Render(),
      text(" Password:"),
      new_pass_input->Render(),
      text(" Confirm password:"),
      new_confirm_input->Render(),
      text(" " + S->error),
      separator(),
      text(" [Enter] create  [Esc] cancel"),
    }) | border | center;
  });
  auto new_component = new_renderer | CatchEvent([&, S](Event event) {
    if (S->dialog != State::Dialog::NewVault) return false;
    if (event == Event::Escape) {
      S->dialog = State::Dialog::None;
      S->show_new = false;
      S->new_name.clear(); S->new_path.clear(); S->new_mount.clear();
      S->new_pass.clear(); S->new_confirm.clear();
      S->error.clear();
      return true;
    }
    if (event == Event::Return) {
      if (S->new_name.empty()) S->new_name = "my-vault";
      if (S->new_path.empty()) S->new_path = "~/Documents/";
      if (S->new_mount.empty()) S->new_mount = "/tmp/" + S->new_name;
      if (S->new_pass.empty()) {
        S->error = "Password cannot be empty.";
        return true;
      }
      if (S->new_pass != S->new_confirm) {
        S->error = "Passwords do not match.";
        return true;
      }
      auto parent = resolve_path(S->new_path);
      auto full_path = (fs::path(parent) / S->new_name).string();
      auto mount_pt = resolve_path(S->new_mount);
      if (Gocryptfs::create(full_path, S->new_pass)) {
        Vault v;
        v.id = store.next_id();
        v.name = S->new_name;
        v.path = full_path;
        v.mount_point = mount_pt;
        store.add(v);
        S->status = "Created vault " + v.name;
      } else {
        S->error = "Failed to create vault. Check directory and permissions.";
        return true;
      }
      S->new_name.clear(); S->new_path.clear(); S->new_mount.clear();
      S->new_pass.clear(); S->new_confirm.clear();
      S->error.clear();
      S->dialog = State::Dialog::None;
      S->show_new = false;
      return true;
    }
    return false;
  });

  // ── Remove Vault Dialog ────────────────────────────────────
  auto remove_renderer = Renderer([&, S](bool) {
    bool mounted = false;
    std::string size_str;
    {
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        mounted = vaults[S->selected].mounted();
        size_str = vault_size_info(vaults[S->selected].path);
      }
    }
    if (S->remove_confirm) {
      return vbox({
        text(" Confirm Delete ") | bold | center,
        separator(),
        text(" Permanently delete \"" + S->remove_name + "\"?") | bold | center,
        size_str.empty() ? text("") : text(" " + size_str) | dim | center,
        text(""),
        text("  ⚠  All files will be lost.") | color(Color::Red) | bold,
        text("  This cannot be undone.") | color(Color::Red),
        text(""),
        separator(),
        text(" [Enter] confirm  [Esc] go back"),
      }) | border | center;
    }
    auto opt1 = S->remove_choice == 0 ? text(" [1] Remove from list only") | inverted
                                      : text(" [1] Remove from list only");
    auto opt2 = S->remove_choice == 1 ? text(" [2] Remove and delete files") | inverted
                                      : text(" [2] Remove and delete files") | color(Color::Red);
    Elements body = {
      text(" Remove Vault ") | bold | center,
      separator(),
      text(" \"" + S->remove_name + "\"") | dim | center,
      size_str.empty() ? text("") : text(" " + size_str) | dim | center,
      separator(),
      opt1,
      opt2,
    };
    if (mounted) {
      body.push_back(text(""));
      body.push_back(text(" ⓘ Vault is mounted — will be unmounted") | color(Color::Yellow));
      body.push_back(text("    before deletion.") | color(Color::Yellow));
    }
    body.push_back(text(""));
    body.push_back(text(" ⚠  This action cannot be reverted.") | color(Color::Red) | bold);
    body.push_back(text("    Make sure you have a backup.") | color(Color::Red));
    body.push_back(separator());
    body.push_back(text(" [1/2] select  [Enter] confirm  [Esc] cancel"));
    return vbox(std::move(body)) | border | center;
  });
  auto remove_component = remove_renderer | CatchEvent([&, S](Event event) {
    if (S->dialog != State::Dialog::RemoveVault) return false;
    if (event == Event::Escape) {
      if (S->remove_confirm) {
        S->remove_confirm = false;
        return true;
      }
      S->dialog = State::Dialog::None;
      S->show_remove = false;
      S->remove_choice = 0;
      S->remove_confirm = false;
      S->remove_name.clear();
      S->remove_path.clear();
      S->remove_mount.clear();
      return true;
    }
    if (!S->remove_confirm) {
      if (event == Event::Character('1')) {
        S->remove_choice = 0;
        return true;
      }
      if (event == Event::Character('2')) {
        S->remove_choice = 1;
        return true;
      }
    }
    if (event == Event::Return) {
      if (S->remove_confirm) {
        Gocryptfs::unmount(S->remove_mount);
        Gocryptfs::remove(S->remove_path);
        Gocryptfs::remove(S->remove_mount);
        store.remove_by_path(S->remove_path);
        if (S->selected >= (int)store.all().size())
          S->selected = std::max(0, (int)store.all().size() - 1);
        S->status = "Removed " + S->remove_name;
        S->dialog = State::Dialog::None;
        S->show_remove = false;
        S->remove_choice = 0;
        S->remove_confirm = false;
        return true;
      }
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        auto& v = vaults[S->selected];
        if (S->remove_choice == 0) {
          if (v.mounted() && !Gocryptfs::unmount(v.mount_point)) {
            S->error = "Failed to unmount " + v.name + ". Is it still in use?";
            S->status = S->error;
            S->dialog = State::Dialog::None;
            S->show_remove = false;
            S->remove_choice = 0;
            return true;
          }
          Gocryptfs::remove(v.mount_point);
          store.remove(v.id);
          if (S->selected >= (int)store.all().size())
            S->selected = std::max(0, (int)store.all().size() - 1);
          S->status = "Removed " + v.name;
          S->dialog = State::Dialog::None;
          S->show_remove = false;
          S->remove_choice = 0;
        } else {
          S->remove_name = v.name;
          S->remove_path = v.path;
          S->remove_mount = v.mount_point;
          S->remove_confirm = true;
        }
      }
      return true;
    }
    return false;
  });

  // ── Change Password Dialog ──────────────────────────────────
  auto change_container = Container::Vertical({
    change_cur_input, change_new_input, change_confirm_input,
  });
  auto change_renderer = Renderer(change_container, [&, S] {
    return vbox({
      text(" Change Password ") | bold | center,
      separator(),
      text(" Current password:"),
      change_cur_input->Render(),
      text(" New password:"),
      change_new_input->Render(),
      text(" Confirm new password:"),
      change_confirm_input->Render(),
      text(" " + S->error),
      separator(),
      text(" [Enter] confirm  [Esc] cancel"),
    }) | border | center;
  });
  auto change_component = change_renderer | CatchEvent([&, S](Event event) {
    if (S->dialog != State::Dialog::ChangePass) return false;
    if (event == Event::Escape) {
      S->dialog = State::Dialog::None;
      S->show_change_pass = false;
      S->change_cur_pass.clear();
      S->change_new_pass.clear();
      S->change_confirm.clear();
      S->error.clear();
      return true;
    }
    if (event == Event::Return) {
      if (S->change_new_pass.empty()) {
        S->error = "New password cannot be empty.";
        return true;
      }
      if (S->change_new_pass != S->change_confirm) {
        S->error = "Passwords do not match.";
        return true;
      }
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        auto& v = vaults[S->selected];
        auto cur = S->change_cur_pass;
        if (cur.empty())
          cur = Keyring::retrieve(v.name);
        if (Gocryptfs::change_password(v.path, cur, S->change_new_pass)) {
          Keyring::remove(v.name);
          S->status = "Password changed. Mount with the new password to store it in keyring.";
        } else {
          S->error = "Failed to change password. Check current password.";
          return true;
        }
      }
      S->change_cur_pass.clear();
      S->change_new_pass.clear();
      S->change_confirm.clear();
      S->error.clear();
      S->dialog = State::Dialog::None;
      S->show_change_pass = false;
      return true;
    }
    return false;
  });

  // ── Main event handler ────────────────────────────────────
  auto main_component = main_renderer | CatchEvent([&, S](Event event) {
    if (S->dialog != State::Dialog::None) return false;

    if (event == Event::Character('q') || event == Event::Character('Q')) {
      screen.Exit();
      return true;
    }
    if (event == Event::Character('j') || event == Event::ArrowDown) {
      const auto& vaults = store.all();
      if (!vaults.empty())
        S->selected = (S->selected + 1) % vaults.size();
      return true;
    }
    if (event == Event::Character('k') || event == Event::ArrowUp) {
      const auto& vaults = store.all();
      if (!vaults.empty())
        S->selected = (S->selected - 1 + vaults.size()) % vaults.size();
      return true;
    }
    if (event == Event::Character('m')) {
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        auto& v = vaults[S->selected];
        auto pw = Keyring::retrieve(v.name);
        if (!pw.empty()) {
          if (Gocryptfs::mount(v.path, v.mount_point, pw)) {
            S->status = "Mounted " + v.name;
          } else {
            S->status = "Failed to mount " + v.name + " — check password";
          }
        } else {
          S->dialog = State::Dialog::MountPassword;
          S->show_mount = true;
          S->pass.clear();
          S->error.clear();
        }
      }
      return true;
    }
    if (event == Event::Character('u')) {
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        auto& v = vaults[S->selected];
        if (Gocryptfs::unmount(v.mount_point))
          S->status = "Unmounted " + v.name;
        else
          S->status = "Failed to unmount " + v.name;
      }
      return true;
    }
    if (event == Event::Character('c')) {
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        auto& v = vaults[S->selected];
        if (v.mounted()) {
          if (!Gocryptfs::unmount(v.mount_point)) {
            S->status = "Failed to unmount " + v.name + " for password change";
            return true;
          }
          S->status = "Unmounted " + v.name + " for password change";
        }
        S->change_cur_pass.clear();
        S->change_new_pass.clear();
        S->change_confirm.clear();
        S->error.clear();
        change_container->SetActiveChild(change_cur_input);
        S->dialog = State::Dialog::ChangePass;
        S->show_change_pass = true;
      }
      return true;
    }
    if (event == Event::Character('v')) {
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        auto& v = vaults[S->selected];
        std::string cmd = "xdg-open \"" + v.path + "\" 2>/dev/null &";
        system(cmd.c_str());
        S->status = "Opened " + v.path;
      }
      return true;
    }
    if (event == Event::Character('o')) {
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        auto& v = vaults[S->selected];
        if (v.mounted()) {
          std::string cmd = "xdg-open \"" + v.mount_point + "\" 2>/dev/null &";
          system(cmd.c_str());
          S->status = "Opened " + v.mount_point;
        } else {
          S->status = "Vault not mounted";
        }
      }
      return true;
    }
    if (event == Event::Character('a')) {
      S->dialog = State::Dialog::AddVault;
      S->show_add = true;
      S->add_path.clear();
      S->add_mount.clear();
      S->error.clear();
      return true;
    }
    if (event == Event::Character('n')) {
      S->dialog = State::Dialog::NewVault;
      S->show_new = true;
      S->new_name.clear(); S->new_path.clear(); S->new_mount.clear();
      S->new_pass.clear(); S->new_confirm.clear();
      S->error.clear();
      return true;
    }
    if (event == Event::Character('r')) {
      const auto& vaults = store.all();
      if (!vaults.empty() && S->selected < (int)vaults.size()) {
        S->remove_name = vaults[S->selected].name;
        S->remove_path = vaults[S->selected].path;
        S->remove_mount = vaults[S->selected].mount_point;
        S->dialog = State::Dialog::RemoveVault;
        S->show_remove = true;
        S->remove_choice = 0;
        S->remove_confirm = false;
      }
      return true;
    }

    return false;
  });

  // ── Assemble with Modals ──────────────────────────────────
  auto component = main_component;
  component |= Modal(mount_component, &S->show_mount);
  component |= Modal(add_component, &S->show_add);
  component |= Modal(new_component, &S->show_new);
  component |= Modal(remove_component, &S->show_remove);
  component |= Modal(change_component, &S->show_change_pass);

  screen.Loop(component);
}

} // namespace guicrypt
