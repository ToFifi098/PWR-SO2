#ifndef P1_SEMAPHORE_H
#define P1_SEMAPHORE_H

#include <mutex>
#include <thread>

class Semaphore {
private:
    int count;
    std::mutex mtx;

public:
    explicit Semaphore(int count) : count(count) {}

    void wait() {
        while (true) {
            std::lock_guard<std::mutex> lock(mtx);
            if (count > 0) {
                --count;
                return;
            }
            std::this_thread::yield();
        }
    }

    void signal() {
        std::lock_guard<std::mutex> lock(mtx);
        ++count;
    }
};

#endif //P1_SEMAPHORE_H
