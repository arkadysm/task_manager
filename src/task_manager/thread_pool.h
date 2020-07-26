#ifndef TASK_MANAGER_THREAD_POOL_H
#define TASK_MANAGER_THREAD_POOL_H

#include "concurrent_queue.h"
#include <atomic>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>

namespace taskman {

class join_threads
{
public:
    explicit join_threads(std::vector<std::thread>& thread_vector)
    : threads(thread_vector) {
    }

    ~join_threads() {
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

private:
    std::vector<std::thread>& threads;

}; // class join_threads

class thread_pool
{
public:
    explicit thread_pool(unsigned thread_count)
    : joiner(threads) {
        try {
            for (unsigned i = 0; i < thread_count; ++i) {
                threads.emplace_back(&thread_pool::operation_thread, this);
            }
        }
        catch (...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
        opqueue.shutdown();
    }

    thread_pool(const thread_pool&) = delete;
    thread_pool(thread_pool&&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    thread_pool& operator=(thread_pool&&) = delete;

    template<typename Callable>
    void submit(Callable&& callable)
    {
        opqueue.emplace(std::forward<Callable>(callable));
    }

private:
    void operation_thread() {
        while (!done) {
            std::function<void()> operation;
            if (opqueue.wait_and_pop(operation)) {
                operation();
            }
        }
    }

    // This order is significant
    std::atomic<bool> done{false};
    concurrent_queue<std::function<void()>> opqueue;
    std::vector<std::thread> threads;
    join_threads joiner;

}; // class thread_pool

} // namespace taskman

#endif // TASK_MANAGER_THREAD_POOL_H
