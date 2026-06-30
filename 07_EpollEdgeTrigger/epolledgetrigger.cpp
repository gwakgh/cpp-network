#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define EPOLL_SIZE 50
#define BUFF_SIZE 4

void set_nonblocking_mode(int fd);

int main(int argc, char* argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_addr;
	socklen_t adr_sz;
	char buf[BUFF_SIZE];

	int epfd, event_cnt;
	struct epoll_event* ep_events;
	struct epoll_event event;

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(9190);

	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)	return 1;
	if(listen(serv_sock, 5) == -1)	return 1;

	std::cout << "Epoll Server Started . . ." << std::endl;

	epfd = epoll_create(EPOLL_SIZE);
	ep_events = new struct epoll_event[EPOLL_SIZE];

	event.events = EPOLLIN;
	event.data.fd = serv_sock;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

	while(1)
	{
		event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);

		if(event_cnt == -1)
		{
			std::cerr << "epoll_wait() error" << std::endl;
			break;
		}

		for(int i = 0; i < event_cnt; i++)
		{
			if(ep_events[i].data.fd == serv_sock)
			{
				adr_sz = sizeof(clnt_addr);
				clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &adr_sz);

				set_nonblocking_mode(clnt_sock);

				event.events = EPOLLIN | EPOLLET;
				event.data.fd = clnt_sock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
				std::clog << "[New Client] Connected fd: " << clnt_sock << std::endl;
			}
			else
			{
				while(1)
				{
					int str_len = read(ep_events[i].data.fd, buf, BUFF_SIZE);

					if(str_len < 0)
					{
						if(errno == EAGAIN)	break;
					}
					else if(str_len == 0)
					{
						epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
						close(ep_events[i].data.fd);
						std::clog << "[Client Disconnected] Closed fd: " << ep_events[i].data.fd << std::endl;
						break;
					}
					else
					{
						write(ep_events[i].data.fd, buf, str_len);
					}
				}
			}
		}
	}

	close(serv_sock);
	close(epfd);
	delete[] ep_events;
	return 0;
}

void set_nonblocking_mode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
