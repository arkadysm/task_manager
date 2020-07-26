#ifndef TASK_MANAGER_CONCURRENT_QUEUE_H
#define TASK_MANAGER_CONCURRENT_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

template<typename T>
class concurrent_queue
{
public:
    concurrent_queue() = default;
    ~concurrent_queue() = default;
    concurrent_queue(const concurrent_queue&) = delete;
    concurrent_queue(concurrent_queue&&) = delete;
    concurrent_queue& operator=(const concurrent_queue&) = delete;
    concurrent_queue& operator=(concurrent_queue&&) = delete;

    bool empty() const {
        std::lock_guard locked(state_mutex);
        return queue_core.empty();
    }

    void push(T element) {
        std::unique_lock locked(state_mutex);
        queue_core.emplace_back(std::move(element));
        locked.unlock();
        state_cond.notify_one();
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        std::unique_lock locked(state_mutex);
        queue_core.emplace_back(std::forward<Args>(args)...);
        locked.unlock();
        state_cond.notify_one();
    }

    bool wait_and_pop(T& element) {
        std::unique_lock locked(state_mutex);
        state_cond.wait(locked, [this] { return !queue_core.empty() || done; });
        return try_pop_impl(element);
    }

    bool try_pop(T& element) {
        std::lock_guard locked(state_mutex);
        return try_pop_impl(element);
    }

    void shutdown() {
        std::unique_lock locked(state_mutex);
        done = true;
        locked.unlock();
        state_cond.notify_all();
    }

private:
    bool try_pop_impl(T& element) {
        if (!queue_core.empty() && !done) {
            element = std::move(queue_core.front());
            queue_core.pop_front();
            return true;
        }
        return false;
    }

private:
    bool done{false};
    std::deque<T> queue_core;
    mutable std::mutex state_mutex;
    std::condition_variable state_cond;

}; // class concurrent_queue

#endif // TASK_MANAGER_CONCURRENT_QUEUE_H
