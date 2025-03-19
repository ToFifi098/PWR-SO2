//
// Created by Filip on 19.03.2025.
//

#ifndef P1_SEMAPHORE_H
#define P1_SEMAPHORE_H


class Semaphore {
private:
    volatile int count;
    volatile bool locked;

public:
    explicit Semaphore(int count) : count(count), locked(false) {}

    void lock() {
        while (true) {
            if (!locked) {
                locked = true;
                return;
            }
            std::this_thread::yield();
        }
    }

    void unlock() {
        locked = false;
    }

    void wait() {
        while (true) {
            lock();
            if (count > 0) {
                count -= 1;
                unlock();
                return;
            }
            unlock();
            std::this_thread::yield();
        }
    }

    void signal() {
        lock();
        count += 1;
        unlock();
    }
};


#endif //P1_SEMAPHORE_H
