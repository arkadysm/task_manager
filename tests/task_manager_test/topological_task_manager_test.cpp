#include <gtest/gtest.h>
#include <thread_pool.h>
#include <topological_task_manager.h>
#include <vector>
#include <functional>

TEST(topological_task_manager_test, run_sequenced_tasks)
{
    std::mutex result_mutex;
    std::vector<int> result;
    
    auto task_impl = [&result_mutex, &result] (int i) {
        std::lock_guard<std::mutex> locked(result_mutex);
        result.push_back(i);
    };

    thread_pool pool(std::thread::hardware_concurrency());
    topological_task_manager task_manager(pool);
    task_manager.add_task(1, std::bind(task_impl, 1), {});
    task_manager.add_task(2, std::bind(task_impl, 2), {1});
    task_manager.add_task(3, std::bind(task_impl, 3), {2});
    task_manager.add_task(4, std::bind(task_impl, 4), {3});
    task_manager.add_task(5, std::bind(task_impl, 5), {4});
    task_manager.run_tasks();

    EXPECT_TRUE((result == std::vector{5,4,3,2,1}));
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
