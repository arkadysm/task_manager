#ifndef TASK_MANAGER_CONCURRENT_QUEUE_H
#define TASK_MANAGER_CONCURRENT_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>

namespace taskman {

template<typename T>
class ConcurrentQueue
{
public:
    template<typename... Args>
    void emplace(Args&&... args) {
        // Awakened thread might sleep again, prevent it
        {
            std::lock_guard locked(m_stateMutex);
            m_queue.emplace_back(std::forward<Args>(args)...);
        }
        m_notEmptyCondition.notify_one();
    }

    T pop() {
        std::unique_lock locked(m_stateMutex);
        m_notEmptyCondition.wait(locked, [this] { return !m_queue.empty(); });
        T element = std::move(m_queue.front());
        m_queue.pop_front();
        return element;
    }

private:
    std::deque<T> m_queue;
    mutable std::mutex m_stateMutex;
    std::condition_variable m_notEmptyCondition;

}; // class ConcurrentQueue

} // namespace taskman

#endif // TASK_MANAGER_CONCURRENT_QUEUE_H
