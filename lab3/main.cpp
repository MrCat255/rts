#include <iostream>
#include <thread>
#include <mutex>
#include <string>

using namespace std;

int coins = 101;
int Bob_coins = 0;
int Tom_coins = 0;

bool Bobs_torn = true;
// int cnt = 0;

mutex mtx;

// Функция дележа монет
void coin_sharing(string name, int& thief_coins, int& companion_coins) {
    while (true) {
        mtx.lock();

        if (coins == 0) {
            mtx.unlock();
            break;
        }

        int total_current = coins + thief_coins + companion_coins;
        if (coins == 1 && (total_current % 2 != 0)) {
            mtx.unlock();
            break;
        }

        // Логика взятия монеты
        if ((Bobs_torn && name == "Bob") || (!Bobs_torn && name == "Tom")) {
            coins--;
            thief_coins++;

            // cnt = 0;

            Bobs_torn = !Bobs_torn;

            cout << name << ": "
                    << thief_coins << " " << companion_coins
                    << " (Осталось: " << coins << ")" << endl;
        }
        // cout << name << " " << cnt << "\n";
        // cnt++;

        mtx.unlock();
        this_thread::yield();
    }
}

int main() {
    thread bob_thread(coin_sharing, "Bob", ref(Bob_coins), ref(Tom_coins));
    thread tom_thread(coin_sharing, "Tom", ref(Tom_coins), ref(Bob_coins));

    bob_thread.join();
    tom_thread.join();

    cout << "\n--- Итоги ---" << endl;
    cout << "Bob: " << Bob_coins << endl;
    cout << "Tom: " << Tom_coins << endl;
    cout << "Осталось покойнику: " << coins << endl;

    system("pause");
    return 0;
}
