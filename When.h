#pragma once

#include "Future.h"

#include <tuple>
#include <utility>
#include <vector>

template<typename R, typename... A>
auto MakeReadyFuture(A&&... args)
{
    Promise<R> promise;
    promise.set_value(R(std::forward<A>(args)...));
    return promise.get_future();
}

template<typename InputIterator>
auto WhenAll(InputIterator begin, InputIterator end) -> Future<std::vector<Future<decltype(begin->get())>>>
{
    using R = decltype(begin->get());
    Future<std::vector<Future<R>>> wait_on_all = MakeReadyFuture<std::vector<Future<R>>>();
    for (auto it = begin; it != end; ++it) {
        Future<R> f = std::move(*it);
        wait_on_all = f.then([w = std::move(wait_on_all)](auto f) mutable {
            auto vec = w.get();
            vec.emplace_back(std::move(f));
            return vec;
        });
    }
    return wait_on_all;
}

Future<std::tuple<>> WhenAll()
{
    return MakeReadyFuture<std::tuple<>>();
}

template<typename FutH, typename... FutT>
auto WhenAll(FutH&& head, FutT&&... tail) -> Future<std::tuple<std::decay_t<FutH>, std::decay_t<FutT>...>>
{
    auto wait_on_tail = WhenAll(std::move(tail)...);

    return head.then([t = std::move(wait_on_tail)](auto h) mutable {
        return std::tuple_cat(std::make_tuple(std::move(h)), t.get());
    });
}
