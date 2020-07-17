#ifndef TASK_MANAGER_THREAD_POOL_H
#define TASK_MANAGER_THREAD_POOL_H

#include "concurrent_queue.h"
#include <atomic>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>

class thread_pool
{
public:
    thread_pool(unsigned threadCount) {
        for (unsigned i = 0; i < threadCount; ++i) {
            threads.emplace_back(&thread_pool::operation_thread, this);
        }
    }

    ~thread_pool() {
        done = true;
        for (auto& t : threads) {
            t.join();
        }
    }

    template<typename Callable>
    void submit(Callable&& callable)
    {
        operationQueue.push(std::forward<Callable>(callable));
    }

private:
    void operation_thread() {
        while (!done) {
            std::function<void()> operation;
            operationQueue.try_pop(operation);
            if (operation) {
                operation();
            }
            else {
                std::this_thread::yield();
            }
        }
    }

    std::atomic<bool> done{false};
    concurrent_queue<std::function<void()>> operationQueue;
    std::vector<std::thread> threads;

}; // class thread_pool

#endif // TASK_MANAGER_THREAD_POOL_H
