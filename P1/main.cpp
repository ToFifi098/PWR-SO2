#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <memory>  // For std::unique_ptr
#include "./Semaphore.h"

// To ensure proper printing
Semaphore stdSemaphore(1);

[[noreturn]] void dine(int philosopher, std::vector<std::unique_ptr<Semaphore>>& forks, Semaphore& diningPhilosophers) {
    int leftFork = philosopher;
    int rightFork = int ((philosopher + 1) % forks.size());

    // Ensure that left fork index is always lower
    if (leftFork > rightFork) {
        std::swap(leftFork, rightFork);
    }

    stdSemaphore.wait();
    std::cout << "Philosopher " << philosopher << " is thinking.\n";
    stdSemaphore.signal();

    while (true) {
        // Thinking phase
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Waiting for available slots for dining
        diningPhilosophers.wait();

        // Locking forks
        forks[leftFork]->wait();
        forks[rightFork]->wait();

        // Eating phase
        stdSemaphore.wait();
        std::cout << "Philosopher " << philosopher << " is eating.\n";
        stdSemaphore.signal();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Done eating, thinking again
        stdSemaphore.wait();
        std::cout << "Philosopher " << philosopher << " is thinking.\n";
        stdSemaphore.signal();

        // Unlock forks and notify other philosophers
        forks[leftFork]->signal();
        forks[rightFork]->signal();

        // Allow another philosopher to eat
        diningPhilosophers.signal();
    }
}

int main(int argc, char* argv[]) {
    // Verification of proper starting argument
    if (argc != 2) {
        std::cerr << "Wrong number of arguments. Only one: Number of philosophers, is accepted\n";
        return 1;
    }

    int philosophersCount = std::stoi(argv[1]);
    if (philosophersCount < 2) {
        std::cerr << "Minimum number of philosophers is 2\n";
        return 1;
    }

    std::vector<std::thread> philosophers;

    // Create vector of unique_ptr to Semaphore
    std::vector<std::unique_ptr<Semaphore>> forks;
    for (int i = 0; i < philosophersCount; ++i) {
        forks.push_back(std::make_unique<Semaphore>(1));
    }

    // To avoid deadlock, one philosopher has to wait to allow others to eat
    Semaphore diningPhilosophers(philosophersCount - 1);

    for (int i = 0; i < philosophersCount; ++i) {
        philosophers.emplace_back(dine, i, std::ref(forks), std::ref(diningPhilosophers));
    }

    for (auto& philosopher : philosophers) {
        philosopher.join();
    }

    return 0;
}
