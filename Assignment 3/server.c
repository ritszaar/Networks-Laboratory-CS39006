#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

/*********************************************\
| Function to find the minimum of two integers |
| Parameters: a - The first integer            |
|             b - The second integer           |
| Returns: The minimum of a and b              |
\*********************************************/
int min(int a, int b) {
    return a < b ? a : b;
}

/*******************************************************************\
| Function to get the current date and time formatted appropiately   |
| Parameters: date_time - The current date and time                  | 
| Returns: void, the formatted date and time is written to date_time |
\*******************************************************************/
void get_date_time(char * date_time) {
    time_t raw_time;
    struct tm * time_info;
    
    time(&raw_time);
    time_info = localtime(&raw_time);

	const char * months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	const char * days[]   = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	int hours = time_info->tm_hour;
	const char * suffix = hours > 12 ? "PM" : "AM";
	if (hours > 12) {
		hours -= 12;		
	} 
    
    sprintf(date_time, "%02d:%02d %s\n%s, %d %s %d", hours, time_info->tm_min, suffix, days[time_info->tm_wday], time_info->tm_mday, months[time_info->tm_mon], time_info->tm_year + 1900);
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

        // printf("\nBytes sent: %d, Bytes left: %d, Total bytes sent: %d\n", bytes_sent, bytes_left, total_bytes_sent);

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
        // printf("\nBytes received: %d, Total bytes received: %d\n", bytes_received, total_bytes_received);
        if (buffer[total_bytes_received - 1] == '\0') {
            break;
        }

    } return total_bytes_received;
}

int main(int argc, char ** argv) {

    /****************\
    | The port number |
    \****************/
    int port;
    if (argc == 1) {
        printf("Error [Unable to get port number]: Run as <executable file> <port>.\n");
        exit(EXIT_FAILURE);
    } else {
        port = atoi(argv[1]);
    }

    srand(port);

	/************\
    | The sockets |
    \************/
	int	sockfd, newsockfd; 

    /********************************************\
    | The server address and the balancer address |
    \********************************************/
	struct sockaddr_in balancer_address, server_address;

    /****************************\
    | The balancer address length |
    \****************************/
    socklen_t balancer_address_length;

	/***********************************\
    | The buffers used for communication |
    \***********************************/
	char buffer[100], request[25];

	/******************\
    | Create the socket |
    \******************/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error [Unable to create socket]:");
		exit(EXIT_FAILURE);
	}

	/**************************\
    | Define the server address |
    \**************************/
	server_address.sin_family		= AF_INET;
	server_address.sin_addr.s_addr	= INADDR_ANY;
	server_address.sin_port		    = htons(port);

	/**************************************\
    | Bind the socket to the server address |
    \**************************************/
	if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		perror("Error [Unable to bind local address]");
		exit(EXIT_FAILURE);
	}

	/*****************\
    | Start the server |
    \*****************/
	listen(sockfd, 5);
	printf("\nServer running on port %d.\n\n", port);

	while (1) {

        /****************************************************************************\
        | Accept an incoming balancer connection request and get the balancer details |
        \****************************************************************************/
        if ((newsockfd = accept(sockfd, (struct sockaddr *) &balancer_address, &balancer_address_length)) < 0) {
            perror("Error [Unable to accept client request]");
            exit(EXIT_FAILURE);
        } 

        receive_all(newsockfd, request, 50);
        
        if (!strcmp(request, "Send Load")) {

            /**********************************************\
            | Send a dummy load for the request "Send Load" |
            \**********************************************/
		    int load = rand() % 100 + 1;
            memset(buffer, '\0', sizeof(buffer));
            sprintf(buffer, "%d", load);
            send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
            printf("\nLoad sent: %d\n", load);
            memset(request, '\0', sizeof(request));
        } else if (!strcmp(request, "Send Time")) {

            /****************************************************************\
            | Send the current date and time for the request "Send Date Time" |
            \****************************************************************/
            memset(buffer, '\0', sizeof(buffer));
            get_date_time(buffer);   
            send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
            printf("\nTime sent: \n%s\n", buffer);
            memset(request, '\0', sizeof(request));
        }
	}

	return 0;
}