#pragma once

#include "Future.h"
#include "PackagedTask.h"
#include "SystemScheduler.h"

#include <utility>

template<typename Func, typename... Args>
auto Async(Func func, Args... args)
     -> Future<decltype(func(std::move(args)...))>
{
    using R = decltype(func(std::move(args)...));
    PackagedTask<R(Args...)> task(std::move(func));
    Future<R> result = task.get_future();

    auto wrapper = [](PackagedTask<R(Args...)>& task, Args&... args) {
        task(std::move(args)...);
    };

    auto bound = std::bind(wrapper, std::move(task), std::move(args)...);
    SystemScheduler().schedule(std::move(bound));
    return result;
}
