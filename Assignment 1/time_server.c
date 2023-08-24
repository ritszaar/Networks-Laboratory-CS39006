#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

/******************************************************************************\
| Function to get the date and time on the local machine formatted appropiately |
\******************************************************************************/
void formatTime(char *output) {
    time_t rawtime;
    struct tm * timeinfo;
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);

	const char * months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	int hours = timeinfo->tm_hour;
	const char * suffix = hours > 12 ? "PM" : "AM";
	if (hours > 12) {
		hours -= 12;		
	} 
    
    sprintf(output, "%02d:%02d %s\n%s, %d %s %d", hours, timeinfo->tm_min, suffix, 
			days[timeinfo->tm_wday], timeinfo->tm_mday, months[timeinfo->tm_mon], timeinfo->tm_year + 1900);
}

int main() {
	// The server socket and the client socket
	int	serverSocket, clientSocket; 

    // The server address and the client address
	struct sockaddr_in clientAddress, serverAddress;

	// The buffer used for communication
	char buf[100];

	// Create the server socket
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannot create socket\n");
		exit(0);
	}

	// Define the server address
	serverAddress.sin_family		= AF_INET;
	serverAddress.sin_addr.s_addr	= INADDR_ANY;
	serverAddress.sin_port		= htons(20000);

	// Bind the server socket to the server address
	if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
		perror("Unable to bind local address\n");
		exit(0);
	}

	// Set the server to start listening on the local address
	listen(serverSocket, 5);

	while (1) {
        int clientAddressLen = sizeof(clientAddress);

		// Accept if a request comes from a client
		if ((clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLen)) < 0) {
			perror("Error: Accept error!\n");
			exit(0);
		}
		
		// Get the time on the local machine, format it and send it to the client
		formatTime(buf);
		send(clientSocket, buf, strlen(buf) + 1, 0);

		// Close the client socket
		close(clientSocket);
	}

	return 0;
}
