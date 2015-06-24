
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

int main()
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
