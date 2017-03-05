#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include  <signal.h>
#include <math.h>

#define BACKLOG 10
#define BUFFER_SIZE 100

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void keyboardInterrupt(int sig) {
	char  c;
    printf("\nTerminate ? [y/n]\n");
   	c = getchar();
    if (c == 'y' || c == 'Y')
    	exit(0);
}

int main(int argc, char** argv) {

	int err_status;
	int errno;
	int socketfd, serverSocket;
	struct addrinfo prep, *results;
	struct sockaddr_in host_addr;
	socklen_t host_addr_size;
	char* PORT = argv[2];

	memset(&prep, 0, sizeof prep); // make sure the struct is empty
	prep.ai_family = AF_INET;     // use IPv4
	prep.ai_socktype = SOCK_STREAM; // TCP stream sockets

	if ((err_status = getaddrinfo(NULL, PORT, &prep, &results)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err_status));
		exit(1);
	}

	socketfd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
	bind(socketfd, results-> ai_addr, results->ai_addrlen);
	listen(socketfd, BACKLOG);

	host_addr_size = sizeof(host_addr);
	serverSocket = accept(socketfd, (struct sockaddr *)&host_addr, &host_addr_size);

	if(serverSocket == -1) {
		fprintf(stderr, "accept() error: %s\n", gai_strerror(errno));
		exit(1);
	} else {
		int len_msg;
		char buffer[BUFFER_SIZE] = {}; // works fine !
		char username[8] = {};

		// send server usename / recv and set client username !
		len_msg = sizeof(argv[1]);
		send(serverSocket, argv[1], len_msg, 0);

		err_status = recv(serverSocket, buffer, BUFFER_SIZE, 0);
		if( err_status == 0) {
			fprintf(stderr, "[-] Failed to get username\n[!] Default username is used instead");
		} else
			strcpy(username, buffer);

		//calculate username padding
		int padding = (unsigned)strlen(argv[1]) - (unsigned)strlen(username); // client - server
		int padding_user = 0;
		int padding_argv = 0;

		if(padding > 0) {
			padding_user = abs(padding);
			padding_argv = 0;
		} else {
			padding_user = 0;
			padding_argv = abs(padding);
		}
		

		// char loop
		while(1) {
			// send
			signal(SIGINT, keyboardInterrupt);
			printf("%.*s" ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET " | ", padding_argv, "              ",argv[1]);
			fgets(buffer, BUFFER_SIZE, stdin);
			len_msg = sizeof(buffer);
			send(serverSocket, buffer, len_msg, 0);

			// recv
			err_status = recv(serverSocket, buffer, BUFFER_SIZE, 0);
			if( err_status == 0) {
				fprintf(stderr, "Connection closed\n");
				exit(1);
			} else
				printf("%.*s" ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET " | %s"
							, padding_user, "              ", username, buffer);
			}
		}
	return 0;
}
