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

typedef struct {
	int id;
	char first_name[20];
	char last_name[20];
} Candidate;

typedef struct {
	char ip[256];
	int port;
	int voted_for;
} Vote;

int n; // Number of candidates
int m = 0; // Number of votes cast so far

Candidate candidates[MAX_CANDIDATES];
Vote T[MAX_VOTES];

void * checker_routine() {
	int votes_received[MAX_CANDIDATES];
	while (1) {
		sleep(2);
		for (int i = 0; i < n; i++) {
			votes_received[i] = 0;
		}

		for (int i = 0; i < m; i++) {
			votes_received[T[i].voted_for - 1]++;
		}

		for (int i = 0; i < n; i++) {
			printf("\nCandidate ID: %d\n", candidates[i].id);
			printf("Candidate Name: %s %s\n", candidates[i].first_name, candidates[i].last_name);
			printf("Votes Received: %d\n", votes_received[i]);
		}
	}

	return NULL;
}

void * client_routine(void * arg) {

	int newsockfd = *((int *)arg);

	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	getpeername(newsockfd, (struct sockaddr*)&addr, &len);
	char * peer_ip = inet_ntoa(addr.sin_addr);
	int peer_port  = ntohs(addr.sin_port);

	for (int i = 0; i < m; i++) {
		if (strcmp(T[i].ip, peer_ip) == 0 && T[i].port == peer_port) {
			return NULL;
		}
	}

	int *vote = malloc(sizeof(int));
	recv(newsockfd, vote, sizeof(int), 0);
	printf("%d\n", *vote);

	int id = *vote;
	if (id >= 1 && id <= n) {
		strcpy(T[m].ip, peer_ip);
		T[m].port = peer_port;
		T[m].voted_for = id;
		m++;
		printf("Client <%s, %d> voted for <%s, %s>.\n", peer_ip, peer_port, candidates[id - 1].first_name, candidates[id - 1].last_name);
	}

	close(newsockfd);
	return NULL;
}

int main(int argc, char *argv[]) {

	printf("Enter number of candidates: ");
	scanf("%d", &n);
	printf("\n");

	for (int i = 0; i < n; i++) {
		candidates[i].id = i + 1;
		printf("Enter name of candidate %d: ", i + 1);
		scanf("%s", candidates[i].first_name);
		scanf("%s", candidates[i].last_name);
	}

	pthread_t checker;
	pthread_create(&checker, NULL, &checker_routine, NULL);

	int sockfd, newsockfd;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("[Error] socket()");
		exit(1);
	}

	struct sockaddr_in server_address, client_address;
	socklen_t client_address_length;

	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(8080);
	inet_aton("127.0.0.1", &server_address.sin_addr);

	if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
		perror("[Error] bind()");
		exit(1);
	}

	listen(sockfd, 5);

	while (1) {
		if ((newsockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_address_length)) < 0) {
            perror("[Error] accept()");
            exit(1);
        }

		int *arg = malloc(sizeof(int));
		*arg = newsockfd;
		pthread_t *client = (pthread_t *)malloc(sizeof(pthread_t));
		pthread_create(client, NULL, &client_routine, arg);
	}

	return 0;
}