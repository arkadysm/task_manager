#include <gtest/gtest.h>
#include <thread_pool.h>
#include <atomic>
#include <thread>

TEST(thread_pool_test, submitting_100_operations)
{
    // TODO: Add correct waiting for tasks completion
    std::atomic<int> counter = 0;
    {
        thread_pool pool(std::thread::hardware_concurrency());
        for (int i = 0; i < 100; ++i) {
            pool.submit([&counter]{
                ++counter;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            });
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    EXPECT_TRUE(counter == 100);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
