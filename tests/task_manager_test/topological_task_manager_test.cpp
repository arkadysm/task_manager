#include <gtest/gtest.h>
#include <thread_pool.h>
#include <topological_task_manager.h>
#include <vector>
#include <functional>
#include <random>

using taskman::ThreadPool;
using taskman::TopologicalTaskManager;

namespace {
std::random_device rdevice;
std::mt19937 rengine(rdevice());
std::uniform_int_distribution<int> uni(1, 10);
void sleepRandomDuration() {
    std::this_thread::sleep_for(std::chrono::milliseconds(uni(rengine)*10));
}
} // namespace

TEST(topological_task_manager_test, run_sequenced_tasks)
{
    std::mutex resultMutex;
    std::vector<int> result;
    
    auto testingTask = [&resultMutex, &result] (int i) {
        sleepRandomDuration();
        std::lock_guard<std::mutex> locked(resultMutex);
        result.push_back(i);
    };

    ThreadPool pool(std::thread::hardware_concurrency());
    TopologicalTaskManager taskManager(pool);
    taskManager.addTask(1, std::bind(testingTask, 1), {});
    taskManager.addTask(2, std::bind(testingTask, 2), {1});
    taskManager.addTask(3, std::bind(testingTask, 3), {2});
    taskManager.addTask(4, std::bind(testingTask, 4), {3});
    taskManager.addTask(5, std::bind(testingTask, 5), {4});
    taskManager.addTask(6, std::bind(testingTask, 6), {5});
    taskManager.runTasks();

    EXPECT_TRUE((result == std::vector{1,2,3,4,5,6}));
}

TEST(topological_task_manager_test, run_parallel_all_tasks)
{
    std::atomic<int> result{0};
    
    auto testingTask = [&result] (int i) {
        sleepRandomDuration();
        ++result;
    };

    ThreadPool pool(std::thread::hardware_concurrency());
    TopologicalTaskManager taskManager(pool);
    taskManager.addTask(1, std::bind(testingTask, 1), {});
    taskManager.addTask(2, std::bind(testingTask, 2), {});
    taskManager.addTask(3, std::bind(testingTask, 3), {});
    taskManager.addTask(4, std::bind(testingTask, 4), {});
    taskManager.addTask(5, std::bind(testingTask, 5), {});
    taskManager.addTask(6, std::bind(testingTask, 6), {});
    taskManager.runTasks();

    EXPECT_TRUE(6 == result);
}

TEST(topological_task_manager_test, run_parallel_middle_tasks)
{
    std::mutex resultMutex;
    std::vector<int> result;
    
    auto testingTask = [&resultMutex, &result] (int i) {
        sleepRandomDuration();
        std::lock_guard<std::mutex> locked(resultMutex);
        result.push_back(i);
    };

    ThreadPool pool(std::thread::hardware_concurrency());
    TopologicalTaskManager taskManager(pool);
    taskManager.addTask(1, std::bind(testingTask, 1), {});
    taskManager.addTask(2, std::bind(testingTask, 2), {1});
    taskManager.addTask(3, std::bind(testingTask, 3), {2});
    taskManager.addTask(4, std::bind(testingTask, 4), {2});
    taskManager.addTask(5, std::bind(testingTask, 5), {3,4});
    taskManager.addTask(6, std::bind(testingTask, 6), {5});
    taskManager.runTasks();

    EXPECT_TRUE((result == std::vector{1,2,3,4,5,6} || result == std::vector{1,2,4,3,5,6}));
}

TEST(topological_task_manager_test, run_parallel_both_ends_tasks)
{
    std::mutex resultMutex;
    std::vector<int> result;
    
    auto testingTask = [&resultMutex, &result] (int i) {
        sleepRandomDuration();
        std::lock_guard<std::mutex> locked(resultMutex);
        result.push_back(i);
    };

    ThreadPool pool(std::thread::hardware_concurrency());
    TopologicalTaskManager taskManager(pool);
    taskManager.addTask(1, std::bind(testingTask, 1), {});
    taskManager.addTask(2, std::bind(testingTask, 2), {});
    taskManager.addTask(3, std::bind(testingTask, 3), {1,2});
    taskManager.addTask(4, std::bind(testingTask, 4), {3});
    taskManager.addTask(5, std::bind(testingTask, 5), {4});
    taskManager.addTask(6, std::bind(testingTask, 6), {4});
    taskManager.runTasks();

    EXPECT_TRUE((result == std::vector{1,2,3,4,5,6} ||
                 result == std::vector{1,2,3,4,6,5} ||
                 result == std::vector{2,1,3,4,5,6} ||
                 result == std::vector{2,1,3,4,6,5}));
}

TEST(topological_task_manager_test, throw_with_cycle_detected)
{
    auto testingTask = [] { };

    ThreadPool pool(std::thread::hardware_concurrency());
    TopologicalTaskManager taskManager(pool);
    taskManager.addTask(1, testingTask, {});
    taskManager.addTask(2, testingTask, {1,5});
    taskManager.addTask(3, testingTask, {2});
    taskManager.addTask(4, testingTask, {3});
    taskManager.addTask(5, testingTask, {4});
    taskManager.addTask(6, testingTask, {5});
    EXPECT_THROW(taskManager.runTasks(), std::logic_error);
}

TEST(topological_task_manager_test, throw_with_unregistered_tasks)
{
    auto testingTask = [] { };

    ThreadPool pool(std::thread::hardware_concurrency());
    TopologicalTaskManager taskManager(pool);
    taskManager.addTask(1, testingTask, {});
    taskManager.addTask(2, testingTask, {});
    taskManager.addTask(3, testingTask, {4});
    EXPECT_THROW(taskManager.runTasks(), std::logic_error);
}

TEST(topological_task_manager_test, throw_with_duplicate_task)
{
    auto testingTask = [] { };

    ThreadPool pool(std::thread::hardware_concurrency());
    TopologicalTaskManager taskManager(pool);
    taskManager.addTask(1, testingTask, {});
    taskManager.addTask(2, testingTask, {});
    EXPECT_THROW(taskManager.addTask(2, testingTask, {4}), std::logic_error);
    taskManager.runTasks();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
