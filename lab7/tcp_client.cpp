#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;


char buffer[1024];

int main() {
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "socket" << endl;
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr << "connect" << endl;
        return 2;
    }

    string user_input;
    while (true) {
        cin >> user_input;
        if (user_input == "exit") {
            cout << "Завершение работы клиента..." << endl;
            break;
        }
        int bytes_sent = send(sock, user_input.c_str(), user_input.length(), 0);
        if (bytes_sent < 0) {
            cerr << "Ошибка отправки" << endl;
            break;
        }
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received == 0) {
            cout << "Сервер закрыл соединение" << endl;
            break;
        } else if (bytes_received < 0) {
            cerr << "Ошибка приема" << endl;
            break;
        }

        // send(sock, message, sizeof(message), 0);
        // recv(sock, buf, sizeof(message), 0);

        cout << buffer << endl;

        for (long unsigned int i = 0; i < sizeof(buffer) - 1; ++i) {
            buffer[i] = 0;
        }
    }

    close(sock);
    return 0;
}
