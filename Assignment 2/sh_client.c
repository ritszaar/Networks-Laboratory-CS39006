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
        if (buffer[total_bytes_received - 1] == '\0') {
            break;
        }
    } return total_bytes_received;
}

int main() {

    /***********\
    | The socket |
    \***********/
	int sockfd;

	/*******************\
    | The server address |
    \*******************/
	struct sockaddr_in server_address;

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

    /**************************\
    | Define the server address |
    \**************************/
	server_address.sin_family	= AF_INET;
	server_address.sin_port	= htons(7575);
	inet_aton("127.0.0.1", &server_address.sin_addr);

    /*****************************************\
    | Connect the socket to the server address |
    \*****************************************/
	if ((connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address))) < 0) {
		perror("Error [Unable to connect to server]");
		exit(EXIT_FAILURE);
	}

    /*********************************************\
    | Receive the "LOGIN: " prompt from the server |
    \*********************************************/
    memset(buffer, '\0', sizeof(buffer));
    receive_all(sockfd, buffer, 50);
    printf("\n%s", buffer);

    /***********************************************************\
    | Get the username from the client and send it to the server |
    \***********************************************************/
    memset(buffer, '\0', sizeof(buffer));
    scanf("%s", buffer); getchar();
    send_all(sockfd, buffer, strlen(buffer) + 1, 50);

    /*****************************************\
    | Receive the login status from the server |
    \*****************************************/
    memset(buffer, '\0', sizeof(buffer));
    receive_all(sockfd, buffer, 50);

    /******************************************************************************\
    | If username exists, get commands from the client and execute it on the server |
    } Otherwise close the connection                                                |
    \******************************************************************************/
    if (!strcmp(buffer, "FOUND\0")) {
        printf("Successfully logged into the server.\n");

        while (1) {

            /**********************************************************\
            | Get the command from the client and send it to the server |
            \**********************************************************/
            printf("\nEnter a command to be executed on the server: ");
            memset(buffer, '\0', sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strlen(buffer) - 1] = '\0';
            send_all(sockfd, buffer, strlen(buffer) + 1, 50);

            /************************************************\
            | Close the connection if "exit" command is given |
            \************************************************/
            if (!strcmp(buffer, "exit\0")) {
                break;
            }

            /************************************************************\
            | Receive the result of the command execution from the server |
            \************************************************************/
            memset(buffer, '\0', sizeof(buffer));
            receive_all(sockfd, buffer, 50);

            /*********************************************************************\
            | Display appropiate output as per the result obtained from the server |
            \*********************************************************************/
            if (!strcmp(buffer, "$$$$\0")) {
                printf("Invalid command!\n");
            } else if (!strcmp(buffer, "####\0")) {
                printf("Error in executing command!\n");
            } else {
                printf("Successfully executed the command on the server. The output is given below: \n\n%s\n", buffer);
            }
        }
    } else {
        printf("Invalid username!\n");    
    }

    /*****************\
    | Close the socket |
    \*****************/
    close(sockfd);

    return 0;
}