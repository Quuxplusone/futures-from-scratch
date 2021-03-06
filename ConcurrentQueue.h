#pragma once

#include <mutex>
#include <queue>
#include <utility>

template<class T>
struct ConcurrentQueue {
    std::queue<T> q_;
    bool aborted_ = false;
    std::mutex mtx_;
    std::condition_variable cv_;

    void push(T t) {
        std::lock_guard<std::mutex> lock(mtx_);
        q_.emplace(std::move(t));
        cv_.notify_one();
    }

    bool try_pop(T& t) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (q_.empty()) {
            return false;
        }
        t = std::move(q_.front());
        q_.pop();
        return true;
    }

    void pop(T& t) {
        std::unique_lock<std::mutex> lock(mtx_);
        while (q_.empty() && !aborted_) cv_.wait(lock);
        if (aborted_) throw "aborted";
        t = std::move(q_.front());
        q_.pop();
    }

    void abort() {
        std::lock_guard<std::mutex> lock(mtx_);
        aborted_ = true;
        cv_.notify_all();
    }
};
