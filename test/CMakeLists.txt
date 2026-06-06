include(FetchContent)

FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

set(TEST_SOURCES
  core/vault_test.cc
  core/vault_store_test.cc
)

add_executable(guicrypt-tui_test ${TEST_SOURCES})
target_link_libraries(guicrypt-tui_test PRIVATE guicrypt-lib gtest_main ${FTXUI_LIBS} nlohmann_json::nlohmann_json pthread)
add_test(NAME guicrypt-tui_test COMMAND guicrypt-tui_test)
