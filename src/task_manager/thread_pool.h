#ifndef TASK_MANAGER_THREAD_POOL_H
#define TASK_MANAGER_THREAD_POOL_H

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
            threads.emplace_back(&thread_pool::operationThread, this);
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
        threadsafeEnqueueOperation(std::forward<Callable>(callable));
    }

private:
    void operationThread() {
        while (!done) {
            std::function<void()> operation = threadsafeDequeueOperation();
            if (operation) {
                operation();
            }
            else {
                std::this_thread::yield();
            }
        }
    }

    void threadsafeEnqueueOperation(std::function<void()> operation) {
        std::lock_guard<std::mutex> locked(queueMutex);
        operationQueue.emplace_back(operation);
    }

    std::function<void()> threadsafeDequeueOperation() {
        std::function<void()> operation;
        std::lock_guard<std::mutex> locked(queueMutex);
        if (!operationQueue.empty()) {
            operation = std::move(operationQueue.front());
            operationQueue.pop_front();
        }
        return operation;
    }

    std::atomic<bool> done{false};
    std::mutex queueMutex;
    std::deque<std::function<void()>> operationQueue;
    std::vector<std::thread> threads;

}; // class thread_pool

#endif // TASK_MANAGER_THREAD_POOL_H
