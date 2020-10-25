#include <gtest/gtest.h>
#include <thread_pool.h>
#include <atomic>
#include <thread>

using taskman::ThreadPool;

TEST(thread_pool_test, submitting_100_operations)
{
    std::atomic<int> counter = 0;
    {
        ThreadPool pool(std::thread::hardware_concurrency());
        for (int i = 0; i < 100; ++i) {
            pool.submit([&counter]{
                ++counter;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            });
        }
    }
    EXPECT_TRUE(counter == 100);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
