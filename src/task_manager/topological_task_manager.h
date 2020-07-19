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

        // Init execution graph by transposing the task graph
        for (auto& [id, node] : task_graph) {
            node.indegree = node.deps.size();
            for (int dep_id : node.deps) {
                execution_graph[dep_id].emplace_back(id);
            }
        }
        total_remaining = task_graph.size();

        // Submit parallel tasks with zero indegree
        std::unique_lock locked(state_mutex);
        for (auto& [id, node] : task_graph) {
            if (0 == node.indegree) {
                submit_one_task(node);
            }
        }

        // Wait for completion of all tasks
        done_cond.wait(locked, [this] {
            return 0 == total_remaining;
        });
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

        node.task_function();

        // Update execution graph and submit dependent tasks with zero indegree
        std::unique_lock locked(state_mutex);
        auto it = execution_graph.find(node.id);
        if (it != execution_graph.end()) {
            for (int dep_id : it->second) {
                auto& dep_node = task_graph[dep_id];
                if (0 == --dep_node.indegree) {
                    submit_one_task(dep_node);
                }
            }
        }
        execution_graph.erase(node.id);
        auto remaining = --total_remaining;
        locked.unlock();

        // Notify if all tasks completed
        if (0 == remaining) {
            done_cond.notify_one();
        }
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
