#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define LISTEN_ADDRESS "127.0.0.1"
#define LISTEN_PORT 4006
#define CONN_ADDRESS "127.0.0.1"
#define CONN_PORT 5000

#define BUFF_SIZE 4096
#define BACKLOG 10

int sockfd;

void close_server()
{
	pid_t wpid;
	int status;

	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	while ((wpid = wait(&status)) > 0)
		;
	exit(0);
}

void die(char *err)
{
	perror(err);
	close_server();
}

void close_client(int client_sock, int server_sock)
{
	shutdown(client_sock, SHUT_RDWR);
	shutdown(server_sock, SHUT_RDWR);
	close(client_sock);
	close(server_sock);
}

void listen_data(int listen_socket, int send_socket)
{
	char buffer[BUFF_SIZE];

	while (1) {
		memset(buffer, 0, BUFF_SIZE);
		if (recv(listen_socket, buffer, BUFF_SIZE, 0) <= 0)
			break;
		if (send(send_socket, buffer, BUFF_SIZE, 0) < 0)
			break;
	}
	close_client(listen_socket, send_socket);
	exit(0);
}

void handle_client(int client_sock)
{
	struct sockaddr_in server;
	int server_sock;

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(CONN_ADDRESS);
	server.sin_port = htons(CONN_PORT);

	if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		close_client(client_sock, server_sock);
	if (connect(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0)
		close_client(client_sock, server_sock);
	if (fork() == 0)
		listen_data(server_sock, client_sock);
	if (fork() == 0)
		listen_data(client_sock, server_sock);

	close(server_sock);
	close(client_sock);
}

void accept_connection()
{
	struct sockaddr_in conn;
	int client_sock, len = sizeof(struct sockaddr_in);

	while (1) {
		if ((client_sock = accept(sockfd, (struct sockaddr *)&conn,
					  (socklen_t *)&len)) < 0) {
			close(client_sock);
			continue;
		}
		handle_client(client_sock);
	}
}

void init_server()
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, close_server);

	struct sockaddr_in server;
	int enable = 1;

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(LISTEN_ADDRESS);
	server.sin_port = htons(LISTEN_PORT);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		die("[ERROR] [socket] ");
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		die("[ERROR] [setsockopt] ");
	if (bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0)
		die("[ERROR] [bind] ");
	if (listen(sockfd, BACKLOG) < 0)
		die("[ERROR] [listen] ");

	accept_connection();
}

int main()
{
	init_server();

	return 0;
}
