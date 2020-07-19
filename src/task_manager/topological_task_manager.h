#ifndef TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H
#define TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H

#include "thread_pool.h"
#include <vector>
#include <unordered_map>
#include <functional>

class topological_task_manager
{
public:
    topological_task_manager(thread_pool& pool)
    : execution_pool(pool) {
    }
    
    ~topological_task_manager() {
    }

    void add_task(int id, std::function<void()> task, std::vector<int> deps) {
        task_graph.try_emplace(id, task_node{id, std::move(task), std::move(deps), 0});
    }

    void run_tasks() {
        // TODO: validate there is no cycle

        // TODO: init execution graph by transposing the task graph

        // TODO: submit tasks with zero indegree

        // TODO: wait for completion of all tasks
    }

private:
    struct task_node
    {
        int id;
        std::function<void()> task_function;
        std::vector<int> deps;
        int indegree;        
    };

    void submit_one_task(task_node& node) {
        execution_pool.submit([this, &node] {
            run_one_task(node);
        });
    }

    void run_one_task(task_node& node) {

        // TODO: invoke task function

        // TODO: update execution graph and submit dependent tasks with zero indegree

        // TODO: notify if all tasks completed
    }

private:
    thread_pool& execution_pool;
    std::mutex state_mutex;
    std::condition_variable done_cond;
    unsigned total_remaining{0};
    std::unordered_map<int, task_node> task_graph;
    std::unordered_map<int, std::vector<int>> execution_graph;

}; // class topological_task_manager

#endif // TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H
