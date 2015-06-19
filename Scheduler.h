#pragma once

#include "ConcurrentQueue.h"
#include <atomic>
#include <functional>
#include <thread>
#include <utility>
#include <vector>

using Task = std::function<void()>;

struct Scheduler {
    std::vector<ConcurrentQueue<Task>> queues_;
    std::vector<std::thread> threads_;
    std::atomic<int> index_ {0};

    Scheduler(int n) : queues_(n) {
        auto do_one_task = [=](int i) {
            Task t;
            for (int j = i; j < i+n; ++j) {
                if (queues_[j % n].try_pop(t)) {
                    return t();
                }
            }
            queues_[i].pop(t);
            t();
        };
        auto do_tasks = [=](int i) {
            try {
                while (true) do_one_task(i);
            } catch (...) {}
        };
        for (int i=0; i < n; ++i) {
            threads_.emplace_back(do_tasks, i);
        }
    }

    ~Scheduler() {
        for (auto& q : queues_) q.abort();
        for (auto& t : threads_) t.join();
    }

    void schedule(Task t) {
        int index = index_++ % queues_.size();
        queues_[index].push(std::move(t));
    }
};
