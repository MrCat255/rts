#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;


int main() {
    int sock, listener;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        cerr << "socket" << endl;
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr << "bind" << endl;
        return 2;
    }

    listen(listener, 1);

    while (true) {
        sock = accept(listener, NULL, NULL);
        if(sock < 0)
        {
            cerr << "accept" << endl;
            return 3;
        }

        while(true)
        {
            bytes_read = recv(sock, buf, 1024, 0);
            if (bytes_read <= 0) break;
            string msgReceived = string(buf, 0, bytes_read);
            if (msgReceived == "finite") {
                send(sock, "server is close", 16, 0);
                close(sock);
                return 0;
            }
            send(sock, buf, bytes_read, 0);
        }

        close(sock);
    }

    return 0;
}
