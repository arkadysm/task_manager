#ifndef TASK_MANAGER_CONCURRENT_QUEUE_H
#define TASK_MANAGER_CONCURRENT_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

template<typename T>
class concurrent_queue
{
public:
    bool empty() const {
        std::lock_guard locked(queue_mutex);
        return queue_core.empty();
    }

    void push(T element) {
        std::unique_lock locked(queue_mutex);
        queue_core.push_back(std::move(element));
        locked.unlock();
        queue_cond.notify_one();
    }

    bool wait_and_pop(T& element) {
        std::unique_lock locked(queue_mutex);
        queue_cond.wait(locked, [this] {
            return !queue_core.empty();
        });
        element = std::move(queue_core.front());
        queue_core.pop_front();
        return true;
    }

    bool try_pop(T& element) {
        std::lock_guard locked(queue_mutex);
        if (!queue_core.empty()) {
            element = std::move(queue_core.front());
            queue_core.pop_front();
            return true;
        }
        return false;
    }

private:
    mutable std::mutex queue_mutex;
    std::condition_variable queue_cond;
    std::deque<T> queue_core;

}; // class concurrent_queue

#endif // TASK_MANAGER_CONCURRENT_QUEUE_H
