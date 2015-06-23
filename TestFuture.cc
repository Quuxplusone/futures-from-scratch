
#include "Future.h"

#include <assert.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

void test_simple_cases()
{
    Promise<int> p;
    Future<int> f = p.get_future();

    try {
        p.get_future();
        assert(false);
    } catch (const char *s) {
        assert(strcmp(s, "future_already_retrieved") == 0);
    }

    Promise<int> p2 = std::move(p);

    try {
        p.get_future();
        assert(false);
    } catch (const char *s) {
        assert(strcmp(s, "no_state") == 0);
    }

    Future<int> f2 = std::move(f);

    try {
        f.get();
        assert(false);
    } catch (const char *s) {
        assert(strcmp(s, "no_state") == 0);
    }

    std::thread t1([f = std::move(f2)]() mutable {
        puts("Getting...");
        assert(f.get() == 42);
        puts("...Gotten.");

        try {
            f.get();
            assert(false);
        } catch (const char *s) {
            assert(strcmp(s, "no_state") == 0);
        }
    });
    sleep(1);
    puts("Setting...");
    p2.set_value(42);
    t1.join();

    try {
        p2.set_value(43);
        assert(false);
    } catch (const char *s) {
        assert(strcmp(s, "promise_already_satisfied") == 0);
    }
}

void test_breaking_promises()
{
    Promise<int> p;
    Future<int> f = p.get_future();

    std::thread t1([f = std::move(f)]() mutable {
        try {
            assert(f.valid() && !f.ready());
            puts("Waiting...");
            f.wait();
            puts("...Done waiting.");
            assert(f.valid() && f.ready());
            puts("Getting...");
            f.get();
            assert(false);
        } catch (const char *s) {
            puts("...Gotten (broken promise).");
            assert(strcmp(s, "broken_promise") == 0);
            assert(!f.valid());
        }
    });
    sleep(1);
    puts("Breaking...");
    p = Promise<int>();
    puts("...Broken.");
    t1.join();
}

void test_preset_promise()
{
    Promise<int> p;
    p.set_value(42);
    int v = [](Promise<int> p){ return p.get_future().get(); }(std::move(p));
    assert(v == 42);
}

struct ThrowingWidget {
    bool b_;
    ThrowingWidget() : b_(false) {}
    ThrowingWidget(bool b) : b_(b) {}
    ThrowingWidget(ThrowingWidget&&) = default;
    ThrowingWidget& operator=(ThrowingWidget&& rhs) {
        if (rhs.b_) throw "throwing_widget";
        return *this;
    }
};

void test_throw_in_set_value()
{
    Promise<ThrowingWidget> p;
    try {
        p.set_value(ThrowingWidget(true));
        assert(false);
    } catch (const char *s) {
        assert(strcmp(s, "throwing_widget") == 0);
    }
    p.set_value(ThrowingWidget(false));
}

int main()
{
    test_simple_cases();
    test_breaking_promises();
    test_preset_promise();
    test_throw_in_set_value();
}
