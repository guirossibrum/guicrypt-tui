#include <gtest/gtest.h>
#include "core/vault.h"

using namespace guicrypt;

TEST(VaultTest, HasCorrectAttributes) {
  Vault v{1, "test", ".", "/tmp"};
  EXPECT_EQ(v.id, 1);
  EXPECT_EQ(v.name, "test");
  EXPECT_EQ(v.path, ".");
  EXPECT_EQ(v.mount_point, "/tmp");
}

TEST(VaultTest, IsValidIfPathExists) {
  Vault v{1, "test", ".", "/tmp"};
  EXPECT_TRUE(v.valid());
}

TEST(VaultTest, IsInvalidIfNameEmpty) {
  Vault v{1, "", ".", "/tmp"};
  EXPECT_FALSE(v.valid());
}
