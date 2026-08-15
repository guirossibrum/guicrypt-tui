# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

guicrypt-tui: a C++20 TUI for managing [gocryptfs](https://github.com/rfjakob/gocryptfs) encrypted vaults on Linux, built with FTXUI. Two-pane layout (vault list + detail panel) with modal dialogs, bottom status/key-hint bar. Targets Arch Linux; auto-installs `gocryptfs` via `pacman` if missing.

A detailed `AGENTS.md` already exists in the repo root with a full architecture/feature breakdown — read it for exhaustive detail beyond what's summarized here.

## Build & Test

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
./build/guicrypt-tui              # run the app
./build/test/guicrypt-tui_test    # run all tests
```

Run a single test by name (GoogleTest filter):

```bash
./build/test/guicrypt-tui_test --gtest_filter=VaultTest.SomeTestName
```

FTXUI, nlohmann-json, and googletest are fetched automatically via CMake `FetchContent`. To use system-installed packages instead, pass `-DUSE_SYSTEM_DEPS=ON`.

## Architecture

Two-layer separation, enforced by convention (not by build boundaries):

- **`src/core/`** — pure logic, no UI dependency, unit-tested in isolation.
  - `vault.h/cc` — `Vault` model (id, name, path, mount_point); `Vault::mounted()` checks `/proc/mounts` directly.
  - `vault_store.h/cc` — JSON persistence of the vault list to `~/.config/guicrypt-tui/vaults.json` (nlohmann-json).
  - `gocryptfs.h/cc` — CLI wrappers for mount/unmount/create/remove/check_installed/change_password.
  - `keyring.h/cc` — password storage via the `secret-tool` CLI (label `"guicrypt-tui"`, attribute `"vault"` = vault name).
- **`src/ui/`** — `screen.cc` is a single-file FTXUI screen containing the whole UI: list/detail panes, status bar, and five modal dialogs (mount password, add vault, new vault, remove vault, change password). The UI layer calls into `core/` and never shells out directly.
- **`src/util/exec.h`** — shared `popen`-based command execution helper used by `core/`.

### FTXUI event flow (screen.cc)

```
Modal (routes to dialog when show=true, else passes through)
  → CatchEvent (global keybindings: j/k, m, u, v, o, a, n, r, c, q)
    → Renderer (main two-pane layout)
```

Key gotcha: FTXUI's `Modal` uses `Container::Tab` internally, so for events to reach the main component while a modal is hidden, the main component must be focusable — use the `Renderer(bool)` overload, not `Renderer()`.

### Non-obvious implementation details

- Mounting/creating a vault requires `mkdir -p` first (mount point / parent dir respectively) — gocryptfs won't create these itself.
- Unmounting always uses `fusermount -uz` (lazy unmount) because a file manager opened via `xdg-open` on the mount point can otherwise block a normal unmount.
- Shell arguments passed to `system()`/`popen()` are quoted with `\"..\"` to handle paths containing spaces; `~` expansion is left to the shell, not done explicitly in C++.
- CMake note: an `OBJECT` library did not propagate FTXUI include paths correctly — `guicrypt-lib` must stay a `STATIC` library.
- Omarchy integration is runtime-detected, not build-time: `guicrypt-tui.desktop` launches via `xdg-terminal-exec --app-id=TUI.float` (Omarchy's Hyprland rules auto-float/center windows with that app-id; the flag is inert on other setups). `Gocryptfs::install()` checks for `omarchy-pkg-add` on `PATH` and prefers it (plus an `omarchy-notification-send` toast) over a raw `sudo pacman` call when present, falling back otherwise.
