#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>

void error_handling(const char* message);
void handle_clnt(int clnt_sock);

int main(int argc, char* argv[])
{
	int serv_sock;
	int clnt_sock;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	if(argc != 2)
	{
		std::cerr << "Usage : " << argv[0] << " <port>" << std::endl;
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)	error_handling("socket() error");

	int toggle = 1;
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &toggle, sizeof(toggle));

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("bind() error");

	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	std::cout << "Multi Thread Server Started . . . " << std::endl;

	while(1)
	{
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if(clnt_sock == -1)	continue;

		std::clog << "Connected client IP: " << inet_ntoa(clnt_addr.sin_addr) << " (socket fc: " << clnt_sock << ")" << std::endl;
		std::thread t(handle_clnt, clnt_sock);

		t.detach();
	}

	close(serv_sock);
	return 0;
}

void handle_clnt(int clnt_sock)
{
	char message[1024];
	int str_len;

	while((str_len = read(clnt_sock, message, sizeof(message) - 1)) != 0)
	{
		if(str_len == -1)	break;

		message[str_len] = '\0';
		std::clog << "Message from client (" << clnt_sock << "): " << message << std::endl;

		write(clnt_sock, message, str_len);
	}

	std::clog << "Disconnected client fd: " << clnt_sock << std::endl;
	close(clnt_sock);
}

void error_handling(const char* message)
{
	std::cerr << message << std::endl;
	exit(1);
}
