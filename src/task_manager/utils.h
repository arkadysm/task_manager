#ifndef TASK_MANAGER_UTILS_H
#define TASK_MANAGER_UTILS_H

#include <mutex>
#include <iostream>

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

template<typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<K,V>& kv) {
    for (const auto& [k, v] : kv) {
        os << k << " => " << v << '\n';
    }
    return os;
}

#endif // TASK_MANAGER_UTILS_H
