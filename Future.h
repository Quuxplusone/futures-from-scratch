#pragma once

#include <exception>
#include <memory>
#include <mutex>
#include <utility>

template<class R> struct Promise;
template<class R> struct Future;

template<class R>
struct SharedState {
    R value_;
    std::exception_ptr exception_;
    bool ready_ = false;
    std::mutex mtx_;
    std::condition_variable cv_;
};

template<class R>
struct Promise {

    std::shared_ptr<SharedState<R>> state_;
    bool future_already_retrieved_ = false;

    Promise() : state_(new SharedState<R>) {}

    Promise(const Promise&) = delete;
    Promise& operator=(const Promise&) = delete;
    Promise(Promise&&) = default;

    Promise& operator=(Promise&& rhs) {
        if (this != &rhs) abandon_state();
        state_ = std::move(rhs.state_);
        return *this;
    }

    ~Promise() { abandon_state(); }

    Future<R> get_future() {
        if (state_ == nullptr) throw "no_state";
        if (future_already_retrieved_) throw "future_already_retrieved";
        future_already_retrieved_ = true;
        return Future<R>(state_);
    }

    void set_value(R r) {
        if (state_ == nullptr) throw "no_state";
        std::lock_guard<std::mutex> lock(state_->mtx_);
        if (state_->ready_) throw "promise_already_satisfied";
        state_->value_ = std::move(r);
        state_->ready_ = true;
        state_->cv_.notify_all();
    }

    void set_exception(std::exception_ptr p) {
        if (state_ == nullptr) throw "no_state";
        std::lock_guard<std::mutex> lock(state_->mtx_);
        if (state_->ready_) throw "promise_already_satisfied";
        state_->exception_ = std::move(p);
        state_->ready_ = true;
        state_->cv_.notify_all();
    }

  private:
    void abandon_state() {
        if (state_ != nullptr) {
            std::lock_guard<std::mutex> lock(state_->mtx_);
            if (!state_->ready_) {
                state_->exception_ = std::make_exception_ptr("broken_promise");
                state_->ready_ = true;
                state_->cv_.notify_all();
            }
        }
    }
};

template<class R>
struct Future {

    std::shared_ptr<SharedState<R>> state_;

    Future() {}
    Future(std::shared_ptr<SharedState<R>> s) : state_(s) {}

    Future(const Future&) = delete;
    Future& operator=(const Future&) = delete;
    Future(Future&&) = default;
    Future& operator=(Future&&) = default;

    R get() {
        wait();
        auto sp = std::move(state_);
        if (sp->exception_) {
            std::rethrow_exception(sp->exception_);
        }
        return std::move(sp->value_);
    }

    bool valid() const {
        return (state_ != nullptr);
    }

    bool ready() const {
        if (state_ == nullptr) return false;
        std::unique_lock<std::mutex> lock(state_->mtx_);
        return state_->ready_;
    }

    void wait() const {
        if (state_ == nullptr) throw "no_state";
        std::unique_lock<std::mutex> lock(state_->mtx_);
        while (!state_->ready_) {
            state_->cv_.wait(lock);
        }
    }
};
