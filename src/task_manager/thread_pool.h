#ifndef TASK_MANAGER_THREAD_POOL_H
#define TASK_MANAGER_THREAD_POOL_H

#include "concurrent_queue.h"
#include <atomic>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>

namespace taskman {

class ThreadPool
{
public:
    explicit ThreadPool(unsigned threadCount): m_joiner(m_threads) {
        try {
            m_threads.reserve(threadCount);
            for (unsigned i = 0; i < threadCount; ++i) {
                m_threads.emplace_back(&ThreadPool::threadPoolMain, this);
            }
        }
        catch (...) {
            shutdown();
            throw;
        }
    }

    ~ThreadPool() {
        shutdown();
    }

    template<typename Callable>
    void submit(Callable&& callable) {
        m_operationQueue.emplace(std::forward<Callable>(callable));
    }

private:
    using Operation = std::function<void()>;
    using ThreadContainer = std::vector<std::thread>;

    class JoinThreads
    {
    public:
        explicit JoinThreads(ThreadContainer& threads): m_threads(threads) {
        }

        ~JoinThreads() {
            for (auto& t : m_threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }

    private:
        ThreadContainer& m_threads;

    }; // class JoinThreads

    void threadPoolMain() {
        bool done = false;
        while (!done) {
            Operation operation = m_operationQueue.pop();
            done = (!!operation) ? operation(), false : true;
        }
    }

    void shutdown() {
        for (auto& t : m_threads) {
            m_operationQueue.emplace(Operation{});
        }
    }

private:
    ConcurrentQueue<Operation> m_operationQueue;
    ThreadContainer m_threads;
    JoinThreads m_joiner;

}; // class ThreadPool

} // namespace taskman

#endif // TASK_MANAGER_THREAD_POOL_H
