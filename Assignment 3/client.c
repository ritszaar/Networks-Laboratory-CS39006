#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 500

/*********************************************\
| Function to find the minimum of two integers |
| Parameters: a - The first integer            |
|             b - The second integer           |
| Returns: The minimum of a and b              |
\*********************************************/
int min(int a, int b) {
    return a < b ? a : b;
}

/*****************************************************************\
| Function to send data to a socket                                |
| Parameters: socket      - The socket to which data is to be sent |
|             buffer      - The buffer to be sent                  |
|             buffer_size - The size of the buffer to be sent      |
|             rate        - The rate at which data is to be sent   |
| Returns: The number of bytes sent, -1 for errors                 |
\*****************************************************************/
int send_all(int socket, char * buffer, size_t buffer_size, size_t rate) {
    int bytes_left = (int) buffer_size, bytes_sent = 0, total_bytes_sent = 0;

    while (bytes_left > 0) {
        if ((bytes_sent = send(socket, buffer + total_bytes_sent, min(rate, bytes_left), 0)) < 0) {
            return -1;
        } 

        total_bytes_sent += bytes_sent;
        bytes_left -= bytes_sent;
    } return bytes_sent;
}

/******************************************************************\
| Function to receive data from a socket                            |
| Parameters: socket - The socket from which data is to be received |
|             buffer - The buffer in which data is to be received   |
|             rate   - The rate at which data is to be received     |
| Returns: The number of bytes received, -1 for errors              |
\******************************************************************/
int receive_all(int socket, char * buffer, size_t rate) {
    int bytes_received = 0, total_bytes_received = 0;
    
    while (1) {
        if ((bytes_received = recv(socket, buffer + total_bytes_received, rate, 0)) < 0) {
            return -1;
        }

        total_bytes_received += bytes_received;
        if (total_bytes_received > 0 && buffer[total_bytes_received - 1] == '\0') {
            break;
        }
    } return total_bytes_received;
}

int main(int argc, char ** argv) {

    /*************************\
    | The balancer port number |
    \*************************/
    int balancer_port;
    if (argc == 1) {
        printf("Error [Unable to get balancer port number]: Run as <executable file> <balancer port>.\n");
        exit(EXIT_FAILURE);
    } else {
        balancer_port = atoi(argv[1]);
    }

    /***********\
    | The socket |
    \***********/
	int sockfd;

	/*********************\
    | The balancer address |
    \*********************/
	struct sockaddr_in balancer_address;

    /**********************************\
    | The buffer used for communication |
    \**********************************/
    char buffer[MAX_BUFFER_SIZE];

    /******************\
    | Create the socket |
    \******************/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error [Unable to create socket]");
		exit(EXIT_FAILURE);
	}

    /****************************\
    | Define the balancer address |
    \****************************/
	balancer_address.sin_family	= AF_INET;
	balancer_address.sin_port	= htons(balancer_port);
	inet_aton("127.0.0.1", &balancer_address.sin_addr);

    /*******************************************\
    | Connect the socket to the balancer address |
    \*******************************************/
	if ((connect(sockfd, (struct sockaddr *) &balancer_address, sizeof(balancer_address))) < 0) {
		perror("Error [Unable to connect to balancer]");
		exit(EXIT_FAILURE);
	}

    memset(buffer, '\0', sizeof(buffer));
    strcpy(buffer, "Send Time\0");

    send_all(sockfd, buffer, strlen(buffer) + 1, 50);
    receive_all(sockfd, buffer, 50);

    printf("\nCurrent date and time is given below: \n%s\n", buffer);

    close(sockfd);

    return 0;
}
