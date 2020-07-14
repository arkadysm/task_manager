#ifndef TASK_MANAGER_THREAD_POOL
#define TASK_MANAGER_THREAD_POOL

#include <atomic>
#include <vector>
#include <thread>

class thread_pool
{
public:
    thread_pool(unsigned threadCount) {
        while (threadCount--) {
            threads.emplace_back(&thread_pool::workerThread, this);
        }
    }

    ~thread_pool() {
        done = true;
        for (auto& t : threads) {
            t.join();
        }
    }

private:
    void workerThread() {
        while (!done) {
            std::this_thread::yield();
        }
    }

    std::atomic<bool> done{false};
    std::vector<std::thread> threads;

}; // class thread_pool

#endif // TASK_MANAGER_THREAD_POOL
