
#include "Scheduler.h"

#include <atomic>
#include <memory>
#include <stdio.h>
#include <thread>
#include <unistd.h>

void test(int n)
{
    Scheduler s(n);
    std::atomic<int> latch(0);
    printf("Testing with n=%d...\n", n);
    for (int i=0; i < 10; ++i) {
        s.schedule([&]{ sleep(1); printf("1 "); fflush(stdout); ++latch; });
        s.schedule([&]{ sleep(2); printf("2 "); fflush(stdout); ++latch; });
        s.schedule([&]{ sleep(3); printf("3 "); fflush(stdout); ++latch; });
    }
    while (latch != 30) { }
    printf("\n");
}

void test_moveonly(int n)
{
    Scheduler s(n);
    std::atomic<int> latch(0);
    printf("Testing (move-only) with n=%d...\n", n);
    for (int i=0; i < 10; ++i) {
        s.schedule([&, p = std::unique_ptr<int>()]{ sleep(1); printf("1 "); fflush(stdout); ++latch; });
        s.schedule([&, p = std::unique_ptr<int>()]{ sleep(2); printf("2 "); fflush(stdout); ++latch; });
        s.schedule([&, p = std::unique_ptr<int>()]{ sleep(3); printf("3 "); fflush(stdout); ++latch; });
    }
    while (latch != 30) { }
    printf("\n");
}

int main()
{
    test_moveonly(20);
    test(3);
    test(8);
    test(16);
    test(32);
}
