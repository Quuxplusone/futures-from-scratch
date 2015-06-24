#pragma once

#include "Future.h"
#include "SystemScheduler.h"

#include <exception>
#include <utility>

template<typename Func, typename R = decltype(std::declval<Func>()())>
Future<R> Async(Func f)
{
    Promise<R> p;
    Future<R> result = p.get_future();
    auto task = [p = std::move(p), f = std::move(f)]() mutable {
        try {
            p.set_value(f());
        } catch (...) {
            p.set_exception(std::current_exception());
        }
    };
    SystemScheduler().schedule(std::move(task));
    return result;
}
