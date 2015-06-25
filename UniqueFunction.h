#pragma once

#include <memory>
#include <utility>

template<typename Signature>
class UniqueFunction;

template<>
class UniqueFunction<void()> {

    struct ContainerBase {
        virtual void call() = 0;
        virtual ~ContainerBase() {}
    };

    template<class F>
    struct Container : ContainerBase {
        F f_;
        Container(F f) : f_(std::move(f)) {}
        virtual void call() { f_(); }
    };

    std::unique_ptr<ContainerBase> ctr_;

  public:
    UniqueFunction() : ctr_(nullptr) {}

    template<class F> UniqueFunction(F f) : ctr_(new Container<F>(std::move(f))) {}

    void operator()() const { return ctr_->call(); }

    operator bool() const { return ctr_ != nullptr; }
};
