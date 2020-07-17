#include <gtest/gtest.h>
#include <thread_pool.h>
#include <utils.h>
#include <iostream>

TEST(thread_pool_test, submitting_100_operations)
{
    thread_pool pool(std::thread::hardware_concurrency());
    for (int i = 0; i < 100; ++i) {
        pool.submit([]{
            cout_lock{}, std::cout << "thread: " << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
    }
    EXPECT_TRUE(true);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
