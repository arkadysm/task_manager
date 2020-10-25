#ifndef TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H
#define TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H

#include "thread_pool.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <stdexcept>

namespace taskman {

class TopologicalTaskManager
{
public:
    TopologicalTaskManager(ThreadPool& threadPool): m_threadPool(threadPool) {
    }
    
    void addTask(int id, std::function<void()> task, std::vector<int> deps) {
        // TODO: investigate actual count of move actions.
        if (!m_taskGraph.try_emplace(id, TaskNode{id, std::move(task), std::move(deps), 0}).second) {
            throw std::invalid_argument("task already registered");
        }
    }

    void runTasks() {

        if (hasUnregisteredTasks())
            throw std::logic_error("unregistered task detected");

        if (hasCycleDependency())
            throw std::logic_error("cycle detected");

        // Init execution graph by transposing the task graph.
        for (auto& [id, node] : m_taskGraph) {
            node.indegree = node.deps.size();
            for (const int dep_id : node.deps) {
                m_executionGraph[dep_id].emplace_back(id);
            }
        }
        m_totalRemaining = m_taskGraph.size();

        // Submit parallel tasks with zero indegree.
        std::unique_lock locked(m_stateMutex);
        for (auto& [id, node] : m_taskGraph) {
            if (0 == node.indegree) {
                submitOneTask(node);
            }
        }

        // Wait for completion of all tasks.
        m_doneCondition.wait(locked, [this] {
            return 0 == m_totalRemaining;
        });
    }

private:
    struct TaskNode
    {
        int id;
        std::function<void()> taskFunction;
        std::vector<int> deps;
        int indegree;        
    };

    void submitOneTask(TaskNode& node) {
        m_threadPool.submit([this, &node] {
            runOneTask(node);
        });
    }

    void runOneTask(TaskNode& node) {

        // TODO: add exception handling and propagate it to run_tasks thread.

        node.taskFunction();

        // Update execution graph and submit dependent tasks with zero indegree.
        std::unique_lock locked(m_stateMutex);
        const auto it = m_executionGraph.find(node.id);
        if (it != m_executionGraph.end()) {
            for (const int dep_id : it->second) {
                auto& dep_node = m_taskGraph[dep_id];
                if (0 == --dep_node.indegree) {
                    submitOneTask(dep_node);
                }
            }
        }
        m_executionGraph.erase(node.id);
        const auto remaining = --m_totalRemaining;
        locked.unlock();

        // Notify if all tasks have been completed.
        if (0 == remaining) {
            m_doneCondition.notify_one();
        }
    }

    bool hasCycleDependency() const {

        // It is Kahn's topological sort.
        // A cycle can be found without transposing the graph.
        std::unordered_map<int, unsigned> indegree;

        for (const auto& [id, node] : m_taskGraph) {
            indegree[id];
            for (const int dep_id : node.deps) {
                ++indegree[dep_id];
            }
        }

        std::vector<int> next;
        next.reserve(m_taskGraph.size());
        for (const auto& [id, remaining] : indegree) {
            if (0 == remaining) {
                next.emplace_back(id);
            }
        }

        unsigned total{0};
        while (!next.empty()) {
            const int id = next.back();
            next.pop_back();

            ++total;

            const auto it = m_taskGraph.find(id);
            if (m_taskGraph.end() != it) {
                for (const int dep_id : it->second.deps) {
                    if (0 == --indegree[dep_id]) {
                        next.emplace_back(dep_id);
                    }
                }
            }
        }

        return total != m_taskGraph.size();
    }

    bool hasUnregisteredTasks() const {
        for (const auto& [id, node] : m_taskGraph) {
            for (const int dep_id : node.deps) {
                if (m_taskGraph.find(dep_id) == m_taskGraph.end()) {
                    return true;
                }
            }
        }
        return false;
    }

private:
    ThreadPool& m_threadPool;
    std::mutex m_stateMutex;
    std::condition_variable m_doneCondition;
    unsigned m_totalRemaining{0};
    std::unordered_map<int, TaskNode> m_taskGraph;
    std::unordered_map<int, std::vector<int>> m_executionGraph;

}; // class TopologicalTaskManager

} // namespace taskman

#endif // TASK_MANAGER_TOPOLOGICAL_TASK_MANAGER_H
