cmake_minimum_required(VERSION 3.20)
project(guicrypt-tui
  VERSION 1.1.0
  DESCRIPTION "TUI manager for gocryptfs encrypted vaults"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ── Dependencies ──────────────────────────────────────────────
option(USE_SYSTEM_DEPS "Use system packages for ftxui and nlohmann-json instead of FetchContent" OFF)

if(USE_SYSTEM_DEPS)
  find_package(ftxui REQUIRED)
  find_package(nlohmann_json REQUIRED)
else()
  include(FetchContent)

  FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG v5.0.0
  )
  FetchContent_MakeAvailable(ftxui)

  FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.3
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

set(FTXUI_LIBS ftxui::screen ftxui::dom ftxui::component)

# ── Library ───────────────────────────────────────────────────
set(LIB_SOURCES
  src/core/vault.cc
  src/core/vault_store.cc
  src/core/gocryptfs.cc
  src/core/keyring.cc
  src/ui/screen.cc
)

add_library(guicrypt-lib STATIC ${LIB_SOURCES})
target_include_directories(guicrypt-lib PUBLIC src)
target_link_libraries(guicrypt-lib PUBLIC ${FTXUI_LIBS} nlohmann_json::nlohmann_json pthread)

# ── Main executable ───────────────────────────────────────────
add_executable(guicrypt-tui src/main.cpp)
target_link_libraries(guicrypt-tui PRIVATE guicrypt-lib ${FTXUI_LIBS} nlohmann_json::nlohmann_json pthread)

# ── Tests ─────────────────────────────────────────────────────
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

# ── Install ───────────────────────────────────────────────────
install(TARGETS guicrypt-tui RUNTIME DESTINATION bin)
install(FILES guicrypt-tui.desktop DESTINATION share/applications)
install(FILES LICENSE DESTINATION share/licenses/guicrypt-tui)
