#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define CLNT_BUF_SIZE 1024

int main(int argc, char* argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char message[CLNT_BUF_SIZE];
    int str_len;

    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(9190);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) 
    {
        std::cerr << "connect() error" << std::endl;
        return 1;
    }

    std::cout << "Connected to Server!" << std::endl;

    while(1)
    {
        std::cout << "Input Message (Q to quit): ";
        std::cin.getline(message, CLNT_BUF_SIZE);

        if (!strcmp(message, "q") || !strcmp(message, "Q")) 
            break;

        write(sock, message, strlen(message));

        str_len = read(sock, message, CLNT_BUF_SIZE - 1);
        if (str_len <= 0) break;

        message[str_len] = '\0';
        std::cout << "Message from server: " << message << std::endl;
    }

    close(sock);
    return 0;
}
