#pragma once

#include "Future.h"
#include "PackagedTask.h"
#include "SystemScheduler.h"

#include <utility>

template<typename Func, typename R = decltype(std::declval<Func>()())>
Future<R> Async(Func f)
{
    PackagedTask<R()> task(std::move(f));
    Future<R> result = task.get_future();
    SystemScheduler().schedule(std::move(task));
    return result;
}
