#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

/*******************************************************************\
| Function to get the current date and time formatted appropiately   |
| Parameters: date_time - The current date and time                  | 
| Returns: void, the formatted date and time is writted to date_time |
\*******************************************************************/
void get_date_time(char * date_time) {
    time_t raw_time;
    struct tm * time_info;
    
    time(&raw_time);
    time_info = localtime(&raw_time);

	const char * months[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
	int hours = time_info->tm_hour;
	const char * suffix = hours > 12 ? "PM" : "AM";
	if (hours > 12) {
		hours -= 12;		
	} 
    
    sprintf(date_time, "%02d:%02d %s\n%s, %d %s %d", hours, time_info->tm_min, suffix, days[time_info->tm_wday], time_info->tm_mday, months[time_info->tm_mon], time_info->tm_year + 1900);
}

int main() {

	/************\
    | The sockets |
    \************/
	int	sockfd, newsockfd; 

    /******************************************\
    | The server address and the client address |
    \******************************************/
	struct sockaddr_in client_address, server_address;

    /**************************\
    | The client address length |
    \**************************/
    socklen_t client_address_length;

	/**********************************\
    | The buffer used for communication |
    \**********************************/
	char date_time[100], request[10];

	/******************\
    | Create the socket |
    \******************/
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Error: Cannot create server socket!\n");
		exit(EXIT_FAILURE);
	}

	/**************************\
    | Define the server address |
    \**************************/
	server_address.sin_family		= AF_INET;
	server_address.sin_addr.s_addr	= INADDR_ANY;
	server_address.sin_port		= htons(9090);

	/**************************************\
    | Bind the socket to the server address |
    \**************************************/
	if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		perror("Error: Unable to bind local address!\n");
		exit(EXIT_FAILURE);
	}

	/*****************\
    | Start the server |
    \*****************/
	printf("\nServer Running....\n");

	while (1) {

        /***************************************************************************************************************************\
        | When the client sends a string "GET", the server gets the client details and sends the current date and time to the client |
        \***************************************************************************************************************************/
		get_date_time(date_time);
        recvfrom(sockfd, request, sizeof(request), 0, (struct sockaddr *) &client_address, &client_address_length);
        
        if (!strcmp(request, "GET")) {
		    sendto(sockfd, date_time, strlen(date_time) + 1, 0, (struct sockaddr *) &client_address, sizeof(client_address));
            memset(request, '\0', sizeof(request));
        }
	}

	return 0;
}