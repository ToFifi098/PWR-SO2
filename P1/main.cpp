#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include "./Semaphore.h"

//To ensure proper printing
Semaphore stdSemaphore(1);

[[noreturn]] void dine(int philosopher, std::vector<Semaphore> &forks, Semaphore diningPhilosophers){
    int leftFork = philosopher;
    int rightFork = (int) ((philosopher + 1) % forks.size());

    //Ensure that lef fork index is always lower
    if (leftFork > rightFork)
        std::swap(leftFork, rightFork);

    stdSemaphore.wait();
    std::cout << "Philosopher " << philosopher << " is thinking.\n";
    stdSemaphore.signal();

    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        //Decrease available count around the table
        diningPhilosophers.wait();

        //Lock forks
        forks[leftFork].wait();
        forks[rightFork].wait();

        stdSemaphore.wait();
        std::cout << "Philosopher " << philosopher << " is eating.\n";
        stdSemaphore.signal();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        //Print then unlock for better print output
        stdSemaphore.wait();
        std::cout << "Philosopher " << philosopher << " is thinking.\n";
        stdSemaphore.signal();

        forks[leftFork].signal();
        forks[rightFork].signal();

        diningPhilosophers.signal();
    }
}

int main(int argc, char* argv[]) {
    // Verification of proper starting argument
    if (argc != 2){
        std::cerr << "Wrong number of arguments, only one: Number of philosophers, is accepted";
    }

    int philosophersCount = std::stoi(argv[1]);
    if (philosophersCount < 2) {
        std::cerr << "Minimum number of philosophers is 2";
    }

    std::vector<std::thread> philosophers;

    std::vector<Semaphore> forks(philosophersCount, Semaphore(1));
    //To avoid deadlock, one philosopher have to wait to allow others to eat
    Semaphore diningPhilosophers(philosophersCount - 1);

    for (int i = 0; i < philosophersCount; ++i) {
        philosophers.emplace_back(dine, i, std::ref(forks), std::ref(diningPhilosophers));
    }

    for (auto &philosopher : philosophers) {
        philosopher.join();
    }

    return 0;
}
