
#include "Async.h"
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

void test_then()
{
    if (true) {
        Promise<int> p;
        p.set_value(42);
        Future<int> f = p.get_future();
        Future<int> f2 = f.then([](Future<int> f){
            assert(f.ready());
            return f.get() + 1;
        });
        assert(!f.valid());
        assert(f2.get() == 43);
    }
    if (true) {
        Promise<int> p;
        Future<int> f = p.get_future();
        p.set_value(42);
        Future<int> f2 = f.then([](Future<int> f){
            assert(f.ready());
            return f.get() + 1;
        });
        assert(!f.valid());
        assert(f2.get() == 43);
    }
    if (true) {
        Promise<int> p;
        Future<int> f = p.get_future();
        Future<int> f2 = f.then([](Future<int> f){
            assert(f.ready());
            return f.get() + 1;
        });
        assert(!f.valid());
        assert(!f2.ready());
        auto g = +[](Future<int> f) {
            return f.get() + 100;
        };
        f2 = f2.then(g);
        p.set_value(0);
        assert(f2.get() == 101);
    }
}

void test_next()
{
    if (true) {
        Promise<int> p;
        p.set_value(42);
        Future<int> f = p.get_future();
        Future<int> f2 = f.next([](int f){ return f+1; });
        assert(!f.valid());
        assert(f2.get() == 43);
    }
    if (true) {
        Promise<int> p;
        Future<int> f = p.get_future();
        p.set_value(42);
        Future<int> f2 = f.next([](int f){ return f+1; });
        assert(!f.valid());
        assert(f2.get() == 43);
    }
    if (true) {
        Promise<int> p;
        Future<int> f = p.get_future();
        Future<int> f2 = f.next([](int f){ return f+1; });
        assert(!f.valid());
        assert(!f2.ready());
        auto g = +[](int f) { return f+100; };
        f2 = f2.next(g);
        p.set_value(0);
        assert(f2.get() == 101);
    }
    if (true) {
        auto inc = [](int f){ return f+1; };
        Promise<int> p;
        Future<int> f = p.get_future();
        Future<int> f2 = f.next(inc).recover([](std::exception_ptr){ return 42; }).next(inc);
        p.set_value(4);
        assert(f2.get() == 6);
    }
    if (true) {
        auto inc = [](int f){ return f+1; };
        Promise<int> p;
        Future<int> f = p.get_future();
        Future<int> f2 = f.next(inc).recover([](std::exception_ptr){ return 42; }).next(inc);
        p.set_exception(std::make_exception_ptr("throw me"));
        assert(f2.get() == 43);
    }
}

void test_canceling_then()
{
    if (true) {
        Future<int> f = Async([]() { sleep(1); puts("done A"); return 0; });
        puts("scheduled A");
        f = f.then([](auto&&) { sleep(1); puts("done B"); return 0; });
        puts("scheduled B");
        f = f.then([](auto&&) { sleep(1); puts("done C"); return 0; });
        puts("scheduled C");
        f.wait();
    }
    if (true) {
        Future<int> f = Async([]() { sleep(1); puts("done A"); return 0; });
        puts("scheduled A");
        f = f.then([](auto&&) { sleep(1); puts("done B"); return 0; });
        puts("scheduled B");
        f = f.then([](auto&&) { assert(false); return 0; });
        puts("scheduled C");
        f = Async([]() { sleep(3); puts("done A2"); return 0; });
        puts("scheduled A2; no longer care about the result of C");
        f.wait();
        puts("done waiting");
        sleep(2);
        puts("done sleeping");
    }
}

void test_multiple_thens()
{
    Future<int> f = Async([]() { sleep(1); puts("done A"); return 1; });
    SharedFuture<int> sf = f.share();
    Future<int> fa = sf.then([](SharedFuture<int> sf) { assert(sf.ready()); puts("done FA"); return sf.get() + 1; });
    Future<int> fb = sf.then([](SharedFuture<int> sf) { assert(sf.ready()); sleep(1); puts("done FB"); return sf.get() + 2; });

    assert(fa.get() == 2);
    assert(sf.ready() && sf.get() == 1);
    assert(sf.ready() && sf.get() == 1);
    assert(!fb.ready());
    assert(fb.get() == 3);
    assert(sf.ready() && sf.get() == 1);
}

int main()
{
    test_simple_cases();
    test_breaking_promises();
    test_preset_promise();
    test_throw_in_set_value();
    test_then();
    test_next();
    test_canceling_then();
    test_multiple_thens();
}
