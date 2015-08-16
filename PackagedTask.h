#pragma once

#include "Future.h"
#include "UniqueFunction.h"

template<class F>
struct PackagedTask;

template<class R, class... A>
struct PackagedTask<R(A...)> {

    UniqueFunction<void(A...)> task_;
    Future<R> future_;
    bool promise_already_satisfied_ = false;

    PackagedTask() = default;

    template<class F>
    PackagedTask(F&& f) {
        Promise<R> p;
        future_ = p.get_future();

        auto f_holder = [f = std::forward<F>(f)]() mutable { return std::move(f); };

        auto sptr = std::make_shared<decltype(f_holder)>(std::move(f_holder));
        std::weak_ptr<decltype(f_holder)> wptr = sptr;

        future_.attach_cancellable_task_state(sptr);

        task_ = [p = std::move(p), wptr = std::move(wptr)](A... args) mutable {
            if (auto sptr = wptr.lock()) {
                auto f = (*sptr)();
                try {
                    p.set_value(f(std::forward<A>(args)...));
                } catch (...) {
                    p.set_exception(std::current_exception());
                }
            }
        };
    }

    bool valid() const { return task_; }

    Future<R> get_future() {
        if (!task_) throw "no_state";
        if (!future_.valid()) throw "future_already_retrieved";
        return std::move(future_);
    }

    void operator()(A... args) {
        if (!task_) throw "no_state";
        if (promise_already_satisfied_) throw "promise_already_satisfied";
        promise_already_satisfied_ = true;
        task_(std::forward<A>(args)...);
    }
};
