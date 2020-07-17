#ifndef TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H
#define TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H

#include "thread_pool.h"
#include <vector>
#include <functional>

class topological_task_manager
{
public:
    topological_task_manager(thread_pool& pool)
    : task_pool(pool) {
    }
    
    ~topological_task_manager() {
    }

    void add_task(int id, std::function<void()> task, std::vector<int> deps) {
    }
    
    void run_tasks() {
    }

private:
    thread_pool& task_pool;

}; // class topological_task_manager

#endif // TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H
