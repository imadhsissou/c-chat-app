#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <math.h>

#define BUFFER_SIZE 100

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


int main(int argc, char** argv) {

	int err_status;
	int socketfd;
	struct addrinfo prep_client, *results_client;
	char* PORT = argv[3];

	memset(&prep_client, 0, sizeof prep_client); // make sure the struct is empty
	prep_client.ai_family = AF_INET;     // use IPv4
	prep_client.ai_socktype = SOCK_STREAM; // TCP stream sockets

	if ((err_status = getaddrinfo(argv[2], PORT, &prep_client, &results_client)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err_status));
		exit(1);
	}

	socketfd = socket(results_client-> ai_family, results_client-> ai_socktype, results_client->ai_protocol);
	printf("Trying %s ...\n",argv[2]);
	err_status = connect(socketfd,results_client->ai_addr, results_client->ai_addrlen);

	if( err_status == -1) {
		fprintf(stderr, "connect() error: %s\n", gai_strerror(errno));
		exit(1);
	} else {
		printf("Connected to %s\n",argv[2]);
		char buffer[BUFFER_SIZE] = {}; // works fine !
		char username[8] = {};

		// recv and set server username / send client username
		err_status = recv(socketfd, buffer, BUFFER_SIZE, 0);
		if( err_status == 0) {
			fprintf(stderr, "[-] Failed to get username, Please retry !\n");
			exit(1);			
		} else {
			strcpy(username, buffer);
			// sending back client username
			int len_msg = sizeof(argv[1]);
			send(socketfd, argv[1], len_msg, 0);
		}
		
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

		// chat loop
		while(1) {

			// recv
			err_status = recv(socketfd, buffer, BUFFER_SIZE, 0);
			if( err_status == 0) {
				fprintf(stderr, "Connection closed\n");
				exit(1);
			} else {
				printf("%.*s" ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET " | %s"
							, padding_user, "              ", username, buffer);
			}	

			// send
			printf("%.*s" ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET " | ", padding_argv, "              ",argv[1]);
			fgets(buffer, BUFFER_SIZE, stdin);
			send(socketfd, buffer, sizeof(buffer), 0);
		}

	}

	return 0;
}
