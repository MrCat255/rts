#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

int dishes[3] = {3000, 3000, 3000};
int eaten_count[3] = {0, 0, 0};
int eating_status[3] = {-1, -1, -1};

enum status {WORKS, FIRED, QUIT, POOR};
enum status cook_status = WORKS;

auto start_time = chrono::steady_clock::now();
const int TIMEOUT_SECONDS = 5;
const int MAX_EAT_LIMIT = 10000;

mutex mtx;

// Функция дележа монет
void cook(int efficiency_cooking) {
    while (true) {
        mtx.lock();
        // Повар больше не работает, завершаем цикл
        if (cook_status != WORKS) {
            mtx.unlock();
            return;
        }

        // Время вышло, повар уволился сам
        auto now = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(now - start_time).count() >= TIMEOUT_SECONDS) {
            cook_status = QUIT;
            mtx.unlock();
            return;
        }

        // Все лопнули, повар не получил зарплату
        if (abs(eating_status[0]) + abs(eating_status[1]) + abs(eating_status[2]) == 0) {
            cook_status = POOR;
            mtx.unlock();
            return;
        }

        // Если все поели, готовим ещё
        if (eating_status[0] >= 0 &&
            eating_status[1] >= 0 &&
            eating_status[2] >= 0)
        {
            for (int i = 0; i < 3; i++) {
                dishes[i] += efficiency_cooking;
            }

            // Разрешаем толстякам есть
            eating_status[0] *= -1;
            eating_status[1] *= -1;
            eating_status[2] *= -1;
        }

        mtx.unlock();
        this_thread::yield();
    }
}

void glutton(int index, int efficiency_eating) {
    while (true) {
        mtx.lock();

        // Повара больше нет, завершаем цикл
        if (cook_status != WORKS) {
            mtx.unlock();
            return;
        }

        // Едим, если еще не ели
        if (eating_status[index] == -1) {
            dishes[index] -= efficiency_eating;
            eaten_count[index] += efficiency_eating;
            eating_status[index] = 1;
            mtx.unlock();
            continue;
        }

        // Мы лопнули, теперь этот поток бесполезен, выходим
        if (eaten_count[index] >= MAX_EAT_LIMIT) {
            eating_status[index] = 0;
            mtx.unlock();
            return;
        }

        // Мы ещё живы, но такелка пуста или отрицательна, повар уволен
        if (dishes[index] <= 0) {
            cook_status = FIRED;
            mtx.unlock();
            return;
        }

        mtx.unlock();
        this_thread::yield();
    }
}

void simulation(
    int efficiency_eating1,
    int efficiency_eating2,
    int efficiency_eating3,
    int efficiency_cooking
) {
    for (int i = 0; i < 3; ++i) {
        dishes[i] = 3000;
        eaten_count[i] = 0;
        eating_status[i] = -1;
    }

    cook_status = WORKS;
    start_time = chrono::steady_clock::now();

    thread cook_copy(cook, efficiency_cooking);
    thread cook_glutton1(glutton, 0, efficiency_eating1);
    thread cook_glutton2(glutton, 1, efficiency_eating2);
    thread cook_glutton3(glutton, 2, efficiency_eating3);
    cook_copy.join();
    cook_glutton1.join();
    cook_glutton2.join();
    cook_glutton3.join();

    //enum status {WORKS, FIRED, QUIT, POOR};
    if (cook_status == WORKS) {
        cout << "Ошибка! Повар почему-то продолжает работать" << endl;
    } else if (cook_status == FIRED) {
        cout << "Повара уволили" << endl;
    } else if (cook_status == QUIT) {
        cout << "Повар уволился сам" << endl;
    } else if (cook_status == POOR) {
        cout << "Повар не получил свою зарплату" << endl;
    } else {
        cout << "Ошибка! Что-то пошло не так" << endl;
    }
    cout << "===ОБСТОЯТЕЛЬСТВА===" << endl;
    cout << "Эффективность поедания равнялась: ";
    cout << efficiency_eating1 << " у первого, ";
    cout << efficiency_eating2 << " у второго, ";
    cout << efficiency_eating3 << " у третьего толстяка." << endl;
    cout << "Эффективность повара: " <<
        efficiency_cooking << "." << endl;
    cout << endl;
}

int main() {

    simulation(1, 10, 10, 5);
    simulation(10000, 10, 10000, 100000);
    simulation(0, 0, 0, 100);

    system("pause");
    return 0;
}
