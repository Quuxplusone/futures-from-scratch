
#include "Future.h"
#include "PackagedTask.h"

#include <assert.h>
#include <stdio.h>
#include <thread>

// http://en.cppreference.com/w/cpp/thread/packaged_task

int f(int x, int y) { return x+y; }

void task_lambda()
{
    PackagedTask<int(int,int)> task([](int a, int b) {
        return a+b;
    });
    Future<int> result = task.get_future();
    task(2, 9);
    printf("%d\n", result.get());
}

void task_bind()
{
    PackagedTask<int()> task(std::bind(f, 2, 11));
    Future<int> result = task.get_future();
    task();
    printf("%d\n", result.get());
}

void task_thread()
{
    PackagedTask<int(int,int)> task(f);
    Future<int> result = task.get_future();
    std::thread task_td(std::move(task), 2, 10);
    task_td.join();
    printf("%d\n", result.get());
}

void test_exceptions()
{
    PackagedTask<int(void)> task([]() -> int {
        puts("should only be called once");
        throw "foo";
    });
    task();
    auto f = task.get_future();
    try {
        task.get_future();
        assert(false);
    } catch (const char *e) {
        assert(strcmp(e, "future_already_retrieved") == 0);
    }
    f.wait();
    assert(task.valid() && f.valid() && f.ready());
    try {
        f.get();
        assert(false);
    } catch (const char *e) {
        assert(strcmp(e, "foo") == 0);
    }
    try {
        task();
        assert(false);
    } catch (const char *e) {
        assert(strcmp(e, "promise_already_satisfied") == 0);
    } catch (...) {
        assert(false);
    }
}

int main()
{
    task_lambda();
    task_bind();
    task_thread();
    test_exceptions();
}
