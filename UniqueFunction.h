#pragma once

#include <memory>
#include <utility>

template<typename Signature>
class UniqueFunction;

template<typename... A>
class UniqueFunction<void(A...)> {

    struct ContainerBase {
        virtual void call(A...) = 0;
        virtual ~ContainerBase() {}
    };

    template<class F>
    struct Container : ContainerBase {
        F f_;
        Container(F f) : f_(std::move(f)) {}
        virtual void call(A... args) { f_(std::move(args)...); }
    };

    std::unique_ptr<ContainerBase> ctr_;

  public:
    UniqueFunction() : ctr_(nullptr) {}

    template<class F> UniqueFunction(F f) : ctr_(new Container<F>(std::move(f))) {}

    void operator()(A... args) const { return ctr_->call(std::move(args)...); }

    operator bool() const { return ctr_ != nullptr; }
};
