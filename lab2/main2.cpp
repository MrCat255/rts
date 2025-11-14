#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>

using namespace std;

void Func(std::string name) {
    long double i = 0;
    auto start = std::chrono::high_resolution_clock::now();
    
    while (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() < 1.0) {
        i += 1e-9; // 10^(-9)
    }
    std::cout << name << ": " << i << std::endl;
}

int main() {
    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    std::thread thread3(Func, "t3");
    
    thread1.join();
    thread2.join();
    thread3.join();

    system("pause"); // Pauses the console on Windows
    
    return 0;
}
