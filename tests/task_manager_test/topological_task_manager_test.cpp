#include <gtest/gtest.h>
#include <thread_pool.h>
#include <topological_task_manager.h>
#include <vector>
#include <functional>
#include <random>

using taskman::thread_pool;
using taskman::topological_task_manager;

namespace {
std::random_device rdevice;
std::mt19937 rengine(rdevice());
std::uniform_int_distribution<int> uni(1, 10);
void random_sleep() {
    std::this_thread::sleep_for(std::chrono::milliseconds(uni(rengine)*10));
}
} // namespace

TEST(topological_task_manager_test, run_sequenced_tasks)
{
    std::mutex result_mutex;
    std::vector<int> result;
    
    auto task_impl = [&result_mutex, &result] (int i) {
        random_sleep();
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
    task_manager.add_task(6, std::bind(task_impl, 6), {5});
    task_manager.run_tasks();

    EXPECT_TRUE((result == std::vector{1,2,3,4,5,6}));
}

TEST(topological_task_manager_test, run_parallel_all_tasks)
{
    std::atomic<int> result{0};
    
    auto task_impl = [&result] (int i) {
        random_sleep();
        ++result;
    };

    thread_pool pool(std::thread::hardware_concurrency());
    topological_task_manager task_manager(pool);
    task_manager.add_task(1, std::bind(task_impl, 1), {});
    task_manager.add_task(2, std::bind(task_impl, 2), {});
    task_manager.add_task(3, std::bind(task_impl, 3), {});
    task_manager.add_task(4, std::bind(task_impl, 4), {});
    task_manager.add_task(5, std::bind(task_impl, 5), {});
    task_manager.add_task(6, std::bind(task_impl, 6), {});
    task_manager.run_tasks();

    EXPECT_TRUE(6 == result);
}

TEST(topological_task_manager_test, run_parallel_middle_tasks)
{
    std::mutex result_mutex;
    std::vector<int> result;
    
    auto task_impl = [&result_mutex, &result] (int i) {
        random_sleep();
        std::lock_guard<std::mutex> locked(result_mutex);
        result.push_back(i);
    };

    thread_pool pool(std::thread::hardware_concurrency());
    topological_task_manager task_manager(pool);
    task_manager.add_task(1, std::bind(task_impl, 1), {});
    task_manager.add_task(2, std::bind(task_impl, 2), {1});
    task_manager.add_task(3, std::bind(task_impl, 3), {2});
    task_manager.add_task(4, std::bind(task_impl, 4), {2});
    task_manager.add_task(5, std::bind(task_impl, 5), {3,4});
    task_manager.add_task(6, std::bind(task_impl, 6), {5});
    task_manager.run_tasks();

    EXPECT_TRUE((result == std::vector{1,2,3,4,5,6} || result == std::vector{1,2,4,3,5,6}));
}

TEST(topological_task_manager_test, run_parallel_both_ends_tasks)
{
    std::mutex result_mutex;
    std::vector<int> result;
    
    auto task_impl = [&result_mutex, &result] (int i) {
        random_sleep();
        std::lock_guard<std::mutex> locked(result_mutex);
        result.push_back(i);
    };

    thread_pool pool(std::thread::hardware_concurrency());
    topological_task_manager task_manager(pool);
    task_manager.add_task(1, std::bind(task_impl, 1), {});
    task_manager.add_task(2, std::bind(task_impl, 2), {});
    task_manager.add_task(3, std::bind(task_impl, 3), {1,2});
    task_manager.add_task(4, std::bind(task_impl, 4), {3});
    task_manager.add_task(5, std::bind(task_impl, 5), {4});
    task_manager.add_task(6, std::bind(task_impl, 6), {4});
    task_manager.run_tasks();

    EXPECT_TRUE((result == std::vector{1,2,3,4,5,6} ||
                 result == std::vector{1,2,3,4,6,5} ||
                 result == std::vector{2,1,3,4,5,6} ||
                 result == std::vector{2,1,3,4,6,5}));
}

TEST(topological_task_manager_test, throw_with_cycle_detected)
{
    auto task_impl = [] { };

    thread_pool pool(std::thread::hardware_concurrency());
    topological_task_manager task_manager(pool);
    task_manager.add_task(1, task_impl, {});
    task_manager.add_task(2, task_impl, {1,5});
    task_manager.add_task(3, task_impl, {2});
    task_manager.add_task(4, task_impl, {3});
    task_manager.add_task(5, task_impl, {4});
    task_manager.add_task(6, task_impl, {5});
    EXPECT_THROW(task_manager.run_tasks(), std::logic_error);
}

TEST(topological_task_manager_test, throw_with_unregistered_tasks)
{
    auto task_impl = [] { };

    thread_pool pool(std::thread::hardware_concurrency());
    topological_task_manager task_manager(pool);
    task_manager.add_task(1, task_impl, {});
    task_manager.add_task(2, task_impl, {});
    task_manager.add_task(3, task_impl, {4});
    EXPECT_THROW(task_manager.run_tasks(), std::logic_error);
}

TEST(topological_task_manager_test, throw_with_duplicate_task)
{
    auto task_impl = [] { };

    thread_pool pool(std::thread::hardware_concurrency());
    topological_task_manager task_manager(pool);
    task_manager.add_task(1, task_impl, {});
    task_manager.add_task(2, task_impl, {});
    EXPECT_THROW(task_manager.add_task(2, task_impl, {4}), std::logic_error);
    task_manager.run_tasks();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
