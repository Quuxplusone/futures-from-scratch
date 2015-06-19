
#include "ConcurrentQueue.h"

#include <stdio.h>
#include <thread>
#include <unistd.h>

int main()
{
    ConcurrentQueue<int> q;
    std::thread t1a([&](){
        q.push(1); sleep(1);
        for (int i=0; i < 20; ++i) q.push(2); sleep(1);
        q.push(3);
    });
    std::thread t1b([&](){
        q.push(4); sleep(1);
        for (int i=0; i < 20; ++i) q.push(5); sleep(1);
        q.push(6);
    });
    std::thread t2([&](){
        for (int i=0; i < 22+22; ++i) {
            int x; q.pop(x); printf("%d ", x); fflush(stdout);
        }
        printf("\n");
    });
    t1a.join(); t1b.join(); t2.join();
}
