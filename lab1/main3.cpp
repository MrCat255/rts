#include <iostream>
#include <time.h>
#include <thread>
#include <mutex>

using namespace std;

void Func(string name)
{
    long long frac = 1;
    for(int i = 0; i < 10000000; ++i)
    {
        long long frac = 1;
        for(long long j = 2; j <= 10; ++j)
        {
            frac *= j;
        }
    }
}

int main() {
    clock_t start = clock();
    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    thread1.join();
    thread2.join();

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    cout << seconds << std::endl;
    
    start = clock();
    
    Func("t1");
    Func("t2");
    
    end = clock();
    seconds = (double)(end - start) / CLOCKS_PER_SEC;
    cout << seconds << std::endl;
    
    system("pause"); // Pauses the console on Windows

    return 0;
}
