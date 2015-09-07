
#include "Async.h"
#include "Future.h"
#include "When.h"

#include <assert.h>
#include <stdio.h>
#include <tuple>
#include <type_traits>
#include <vector>
#include <unistd.h>

void test_when_all()
{
    if (true) {
        auto none = WhenAll();

        assert(none.ready());
        std::tuple<> none_futures = none.get();
        (void)none_futures;
    }
    if (true) {
        Future<int> f = Async([](){ sleep(1); puts("done f"); return 1; });
        Future<double> g = Async([](){ sleep(2); puts("done g"); return 2.5; });
        Future<int> h = Async([](){ sleep(1); puts("done h"); return 3; });

        auto fgh = WhenAll(f, g, h);

        assert(!fgh.ready());

        auto fgh_futures = fgh.get();

        static_assert(std::is_same<
            decltype(fgh_futures),
            std::tuple<decltype(f), decltype(g), decltype(h)>
        >::value, "");

        assert(std::get<0>(fgh_futures).ready());
        assert(std::get<1>(fgh_futures).ready());
        assert(std::get<2>(fgh_futures).ready());

        assert(std::get<0>(fgh_futures).get() == 1);
        assert(std::get<1>(fgh_futures).get() == 2.5);
        assert(std::get<2>(fgh_futures).get() == 3);
    }
    if (true) {
        std::list<Future<int>> fgh_original;

        auto fgh = WhenAll(fgh_original.begin(), fgh_original.end());

        assert(fgh.ready());
        assert(fgh.get().empty());
    }
    if (true) {
        std::list<Future<int>> fgh_original;
        fgh_original.push_back(Async([](){ sleep(1); puts("done f"); return 1; }));
        fgh_original.push_back(Async([](){ sleep(2); puts("done g"); return 2; }));
        fgh_original.push_back(Async([](){ sleep(1); puts("done h"); return 3; }));

        auto fgh = WhenAll(fgh_original.begin(), fgh_original.end());

        assert(!fgh.ready());

        auto fgh_futures = fgh.get();

        static_assert(std::is_same<
            decltype(fgh_futures),
            std::vector<Future<int>>
        >::value, "");

        assert(fgh_futures[0].ready());
        assert(fgh_futures[1].ready());
        assert(fgh_futures[2].ready());

        assert(fgh_futures[0].get() == 1);
        assert(fgh_futures[1].get() == 2);
        assert(fgh_futures[2].get() == 3);
    }
}

int main()
{
    test_when_all();
}
