#include <gtest/gtest.h>

TEST(topological_task_manager_test, empty_test)
{
    EXPECT_TRUE(true);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
