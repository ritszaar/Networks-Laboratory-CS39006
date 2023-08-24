#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
	// The client socket
	int clientSocket;

	// The server address
	struct sockaddr_in	serverAddress;
	
	// Create the client socket
	if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error: Unable to create client socket!\n");
		exit(0);
	}

	// Define the server address
	serverAddress.sin_family	= AF_INET;
	inet_aton("127.0.0.1", &serverAddress.sin_addr);
	serverAddress.sin_port	= htons(20000);

	// Connect the client socket to the server address
	if ((connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) < 0) {
		perror("Error: Unable to connect to server!\n");
		exit(0);
	}

	// Receive the current date and time from the server and display it onto the console
	char buf[100];
	recv(clientSocket, buf, 100, 0);
	printf("%s\n", buf);
		
	// Close the client socket
	close(clientSocket);

	return 0;
}

