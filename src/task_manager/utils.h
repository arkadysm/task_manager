#ifndef TASK_MANAGER_UTILS_H
#define TASK_MANAGER_UTILS_H

#include <mutex>

class cout_lock
{
public:
    cout_lock() {
        get_mutex().lock();
    }

    ~cout_lock() {
        get_mutex().unlock();
    }

private:
    static std::mutex& get_mutex() {
        static std::mutex cout_mutex;
        return cout_mutex;
    }

}; // class cout_lock

#endif // TASK_MANAGER_UTILS_H
