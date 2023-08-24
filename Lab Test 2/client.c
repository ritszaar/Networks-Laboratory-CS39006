#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#define MAX_CANDIDATES 10
#define MAX_VOTERS 500
#define MAX_VOTES 500

int main(int arc, char * argv[]) {
	int sockfd;
	struct sockaddr_in server_address;
    char buffer[4096];

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error [Unable to create socket]");
		exit(1);
	}

	server_address.sin_family	= AF_INET;
	server_address.sin_port	= htons(8080);
	inet_aton("127.0.0.1", &server_address.sin_addr);

	if ((connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address))) < 0) {
		perror("[Error] connect()");
		exit(1);
	}


	int *vote = malloc(sizeof(int));
	printf("Enter ID of candidate to vote for: ");
	scanf("%d", vote);
	if (send(sockfd, vote, sizeof(int), 0) == -1) {
		printf("[Error] send()");
		exit(1);
	}

	return 0;
}