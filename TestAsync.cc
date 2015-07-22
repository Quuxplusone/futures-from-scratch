
#include "Async.h"
#include "Future.h"
#include "Scheduler.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

int f1()
{
    sleep(1);
    puts("Returning...");
    return 42;
}

int f2()
{
    sleep(1);
    puts("Throwing...");
    throw 42;
    return 1;
}

void test_simple_cases()
{
    Future<int> r = Async(f1);
    puts("Getting...");
    assert(r.get() == 42);
    puts("Gotten.");

    r = Async(f2);
    try {
        puts("Getting...");
        r.get();
        assert(false);
    } catch (int e) {
        assert(e == 42);
        puts("Gotten (exception).");
    }
}

void test_move_only_types()
{
    puts("Testing move-only types...");
    auto f3 = [x = std::unique_ptr<int>{}](std::unique_ptr<int> y) -> bool { sleep(2); return x == y; };
    Future<bool> r3 = Async(std::move(f3), std::unique_ptr<int>{});
    Future<bool> r4 = Async(std::move(f3), std::unique_ptr<int>(new int));
    assert(r3.get() == true);
    assert(r4.get() == false);
}

int main()
{
    test_simple_cases();
    test_move_only_types();
}
