# guicrypt-tui

A TUI application for managing [gocryptfs](https://github.com/rfjakob/gocryptfs) encrypted vaults on Linux.

Built with C++20 and [FTXUI](https://github.com/ArthurSonzogni/FTXUI) v5.0.0.

## Features

- Two-pane FTXUI layout with vault list and detail panel
- Create, add, mount, unmount, remove vaults
- Password storage in system keyring (gnome-keyring via secret-tool)
- Auto-detect mounted status (● / ○)
- Modal dialogs for password input, add vault, new vault, remove vault
- Open vault directory or mount point via xdg-open
- Auto-installs gocryptfs if missing (pacman)
- 8 unit tests

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
./build/guicrypt-tui
```

### Run tests

```bash
./build/test/guicrypt-tui_test
```

## Usage

| Key | Action |
|-----|--------|
| `j`/`k` or `↑`/`↓` | Navigate vault list |
| `m` | Mount selected vault |
| `u` | Unmount selected vault |
| `v` | Open vault directory |
| `o` | Open mounted vault |
| `a` | Add existing vault |
| `n` | Create new vault |
| `c` | Change password |
| `r` | Remove vault |
| `q` | Quit |

## Dependencies

- gocryptfs
- gnome-keyring (or compatible secret-tool provider)
- CMake 3.20+, C++20 compiler
- libsecret (runtime, for secret-tool CLI)
- FTXUI, nlohmann-json, googletest (auto-fetched by CMake)

## License

MIT
