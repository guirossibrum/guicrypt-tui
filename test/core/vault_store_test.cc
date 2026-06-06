#include <gtest/gtest.h>
#include <cstdlib>
#include "core/vault_store.h"

using namespace guicrypt;

class VaultStoreTest : public ::testing::Test {
protected:
  std::string orig_home;
  std::string test_dir;

  void SetUp() override {
    orig_home = std::getenv("HOME") ? std::getenv("HOME") : "";
    test_dir = "/tmp/guicrypt-test-vaults";
    system(("rm -rf " + test_dir).c_str());
    setenv("HOME", test_dir.c_str(), 1);
  }

  void TearDown() override {
    setenv("HOME", orig_home.c_str(), 1);
    system(("rm -rf " + test_dir).c_str());
  }
};

TEST_F(VaultStoreTest, StartsEmpty) {
  VaultStore store;
  EXPECT_TRUE(store.all().empty());
}

TEST_F(VaultStoreTest, AddAndFind) {
  VaultStore store;
  store.add({1, "test", ".", "/tmp"});
  EXPECT_EQ(store.all().size(), 1);
  auto v = store.find(1);
  ASSERT_TRUE(v.has_value());
  EXPECT_EQ(v->name, "test");
}

TEST_F(VaultStoreTest, NextIdIncrements) {
  VaultStore store;
  EXPECT_EQ(store.next_id(), 1);
  store.add({1, "v1", ".", "/tmp"});
  EXPECT_EQ(store.next_id(), 2);
  store.add({2, "v2", ".", "/tmp"});
  EXPECT_EQ(store.next_id(), 3);
}

TEST_F(VaultStoreTest, Remove) {
  VaultStore store;
  store.add({1, "v1", ".", "/tmp"});
  store.add({2, "v2", ".", "/tmp"});
  store.remove(1);
  EXPECT_EQ(store.all().size(), 1);
  EXPECT_FALSE(store.find(1).has_value());
  EXPECT_TRUE(store.find(2).has_value());
}

TEST_F(VaultStoreTest, PersistsToDisk) {
  {
    VaultStore store;
    store.add({1, "persist-test", ".", "/tmp"});
  }
  {
    VaultStore store;
    EXPECT_EQ(store.all().size(), 1);
    auto v = store.find(1);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->name, "persist-test");
  }
}
