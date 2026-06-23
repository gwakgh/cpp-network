#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

int main(int argc, char* argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    struct timeval timeout;
    socklen_t adr_sz;
    char buf[1024];

    fd_set reads, cpy_reads;
    int fd_max, fd_num, i;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(9190);

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        std::cerr << "bind() error" << std::endl;
        return 1;
    }
    if (listen(serv_sock, 5) == -1) {
        std::cerr << "listen() error" << std::endl;
        return 1;
    }

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max = serv_sock;

    std::cout << "Select Multiplexing Server Started (Port: 9190) . . ." << std::endl;

    while(1)
    {
        cpy_reads = reads;

        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout);

        if (fd_num == -1) 
	{
            std::cerr << "select() error" << std::endl;
            break;
        }

        if (fd_num == 0) 
	{
            std::clog << "Timeout... No activity from clients." << std::endl;
            continue;
       	}

        for (i = 0; i < fd_max + 1; i++)
        {
            if (FD_ISSET(i, &cpy_reads))
            {
                if (i == serv_sock) 
                {
                    adr_sz = sizeof(clnt_addr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &adr_sz);
                    
                    FD_SET(clnt_sock, &reads);

                    if (fd_max < clnt_sock) 
		    {
                        fd_max = clnt_sock;
                    }
                    std::clog << "[New Client] Connected on socket fd: " << clnt_sock << std::endl;
                }
                else 
                {
                    int str_len = read(i, buf, sizeof(buf) - 1);
                    
                    if (str_len == 0) 
                    {
                        FD_CLR(i, &reads);
                        close(i);
                        std::clog << "[Client Disconnected] Closed socket fd: " << i << std::endl;
                    }
                    else 
                    {
                        buf[str_len] = '\0';
                        std::clog << "Message from fd (" << i << "): " << buf;
                        write(i, buf, str_len);
                    }
                }
            }
        }
    }

    close(serv_sock);
    return 0;
}
