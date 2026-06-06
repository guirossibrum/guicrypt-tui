# AGENTS.md

------------------------------------------------------------
Chapter 1 - PROJECT OVERVIEW
------------------------------------------------------------

App Name: guicrypt-tui
Purpose: Manage gocryptfs encrypted vaults from a TUI interface.
Language: C++ (C++20)
Build System: CMake with FetchContent
TUI Library: FTXUI v5.0.0
Distribution: Arch Linux AUR (planned). Auto-installs gocryptfs if missing.
Interface: Two-pane FTXUI layout with modal dialogs and bottom status/key-hint bar.

Core Features (all implemented and working):
- Maintain a vaults.json list of known vaults at ~/.config/guicrypt-tui/vaults.json.
- Add existing vaults (path + mount point).
- Create new vaults (gocryptfs -init with mkdir -p).
- Mount (mkdir -p + gocryptfs, lazy unmount with fusermount -uz).
- Unmount (fusermount -uz, works even if file manager has mount point open).
- Auto-detect mounted status via /proc/mounts (● bold = mounted, ○ = unmounted).
- Store only id, name, path, mount_point in JSON.
- Use system keyring (secret-tool/gnome-keyring) for password storage.
- Remove vault from list.
- Open vault path or mount point via xdg-open.
- Modal password prompt when no keyring entry exists.
- Auto-create mount point directory before mounting.
- 8 unit tests all passing (Vault x3, VaultStore x5).


------------------------------------------------------------
Chapter 2 - ARCHITECTURE
------------------------------------------------------------

The project follows a modular structure with independent layers:

guicrypt-tui/
├── src/
│   ├── main.cpp                 - Entry point, gocryptfs check, FTXUI Screen launch
│   ├── core/
│   │   ├── vault.h/cc           - Vault model (id, name, path, mount_point)
│   │   ├── vault_store.h/cc     - JSON persistence (vaults.json read/write)
│   │   ├── gocryptfs.h/cc       - CLI wrappers: mount, unmount, create, remove, check_installed, change_password
│   │   ├── keyring.h/cc         - secret-tool integration for password storage
│   │   └── vault.h/cc           - Mounted detection via /proc/mounts in Vault::mounted()
│   ├── ui/
│   │   └── screen.h/cc          - Single-file FTXUI screen with all panels + modals
│   └── util/
│       └── exec.h               - Shared popen-based command execution
├── test/
│   ├── CMakeLists.txt
│   └── core/
│       ├── vault_test.cc        - 3 Vault model unit tests
│       └── vault_store_test.cc  - 5 VaultStore unit tests (CRUD, persistence, next_id)
├── CMakeLists.txt               - Root build config (FetchContent: FTXUI, nlohmann-json, googletest)
├── README.md
└── LICENSE

Separation of concerns:
- Core layer: pure logic, no UI dependency. Testable in isolation.
- UI layer: FTXUI components, calls core services. Never runs system commands directly.
- Util: shared helpers.

No Ruby files remain. All previous Ruby code (lib/, bin/, Gemfile, test/*.rb, etc.) removed.


------------------------------------------------------------
Chapter 3 - BUILD & DEPENDENCIES
------------------------------------------------------------

Dependencies (managed by CMake FetchContent or system packages):
- FTXUI v5.0.0 (TUI framework) - auto-fetched via FetchContent
- nlohmann/json v3.11.3 (JSON parsing) - auto-fetched via FetchContent
- gocryptfs (external, system package - auto-installed if missing)
- libsecret (keyring access, uses secret-tool CLI)
- googletest v1.14.0 (testing) - auto-fetched via FetchContent
- pthread (for async operations if needed)

Build:
  cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
  cmake --build build -j$(nproc)
  ./build/guicrypt-tui

Test:
  ./build/test/guicrypt-tui_test

Arch AUR packaging:
  PKGBUILD depends on: gocryptfs, ftxui, nlohmann-json, libsecret
  Installs to /usr/bin/guicrypt-tui


------------------------------------------------------------
Chapter 4 - UI ARCHITECTURE (FTXUI)
------------------------------------------------------------

Single-screen design in src/ui/screen.cc:

- Main component: Renderer(bool) for two-pane layout (vault list + detail panel) + status bar.
  Uses focusable Renderer variant (takes bool param) to keep the Modal focus chain intact.
- Left pane: vault list with selection highlight (inverted) and mount indicator (●/○).
- Right pane: selected vault details + vault-specific action hints.
- Bottom bar: status message (left), global keybinding hints (right), dark blue bg.

Five modal dialogs (overlaid via Modal decorator):
- Mount password: single Input (password mode) + CatchEvent for Enter/Escape.
- Add vault: path + mount_point Inputs + CatchEvent for Enter/Escape.
- New vault: name, parent dir, mount point, password, confirm Inputs + CatchEvent.
- Remove vault: two radio options (soft/hard delete) + confirm step for hard delete.
- Change password: old password, new password, confirm inputs + CatchEvent.

Modal implementation detail: FTXUI's Modal uses Container::Tab internally. For events to
flow correctly through Modal when hidden, the main component must be Focusable. Use
`Renderer(bool)` (not `Renderer()`) to create a focusable main renderer.

Keybindings:
  Global (no dialog open): j/k/↑↓ navigate, m mount, u unmount, v vault directory,
  o open mount, a add vault, n new vault, r remove, c change password, q quit.
  Dialog: Enter confirm, Esc cancel. Tab cycles input fields.

Event flow:
  Modal (routes to dialog when show=true, else passes through)
    → CatchEvent (main keybindings)
      → Renderer (main layout)


------------------------------------------------------------
Chapter 5 - KEY IMPLEMENTATION NOTES
------------------------------------------------------------

- gocryptfs mount requires mkdir -p for the mount point before mounting.
- gocryptfs create requires mkdir -p for the parent directory.
- fusermount -uz (lazy unmount) required because xdg-opened file manager keeps mount open.
- All shell arguments are quoted with \" to handle paths with spaces.
- Tilde (~) in paths: shell expands ~ in arguments passed to system(); no explicit expansion needed.
- Keyring: secret-tool CLI with label "guicrypt-tui" and attribute "vault" (value = vault name).
- CMake OBJECT library didn't propagate FTXUI includes; STATIC library used instead.

Build commands:
  cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
  cmake --build build -j$(nproc)
  ./build/test/guicrypt-tui_test
  ./build/guicrypt-tui


------------------------------------------------------------
Chapter 6 - DEVELOPMENT PROCESS
------------------------------------------------------------

- Functions should be single-responsibility.
- Use Neovim to navigate/edit, LazyGit to commit, Yazi to browse files.
- Prefer modern C++ (C++20): smart pointers, std::optional, std::filesystem.
- Don't over-engineer — keep it simple and working.
- Test incrementally after each module.
- Always run full test suite after changes.
