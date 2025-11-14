#include <iostream>
#include <time.h>

using namespace std;

int main() {
    clock_t start = clock();
    for(int i = 0; i < 10000000; ++i)
    {
        long long frac = 1;
        for(long long j = 2; j <= 10; ++j)
        {
            frac *= j;
        }
    }

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    cout << seconds << std::endl;

    return 0;
}
