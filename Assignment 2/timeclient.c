#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/poll.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
int main() { 

    /***********\
    | The socket |
    \***********/
    int sockfd; 

    /*******************\
    | The server address |
    \*******************/
    struct sockaddr_in server_address; 

    /**************************\
    | The server address length |
    \**************************/
    socklen_t server_address_length;
  
    /******************\
    | Create the socket |
    \******************/
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        perror("Error [Could not create client socket]: "); 
        exit(EXIT_FAILURE); 
    } 
      
    /**************************\
    | Define the server address |
    \**************************/
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(9090); 
    inet_aton("127.0.0.1", &server_address.sin_addr); 

    int tries = 0;
    char date_time[100];

    const char * num_suffix[] = {"st", "nd", "rd", "th", "th"};

    /*********************************\
    | Define the variables for polling |
    \*********************************/
    struct pollfd fd;
    int poll_result, received = 0;
    fd.fd = sockfd;
    fd.events = POLLIN;

    while (1) {

        /*************************************\
        | Send the message "GET" to the server |
        \*************************************/
        const char * request = "GET";
        printf("\nGetting the current date and time from the server [%d%s try]...\n", tries + 1, num_suffix[tries]);
        sendto(sockfd, request, strlen(request), 0, (const struct sockaddr *) &server_address, sizeof(server_address));

        /*********************************************\
        | Wait for 3 seconds before proceeding further |
        \*********************************************/
        poll_result = poll(&fd, 1, 3000);

        switch (poll_result) {
            case -1:

                /*********************\
                | Exit if pollig fails |
                \*********************/
                perror("Error [Polling unsuccessful]");
                exit(EXIT_FAILURE);
            case 0:

                /**********************************************\
                | If 3 seconds have passed, increase tries by 1 |
                \**********************************************/
                tries++;
                break;
            default:

                /*********************************************************************************\
                | Receive the current date and time from the server and display it to the terminal |
                \*********************************************************************************/
                if ((recvfrom(sockfd, date_time, sizeof(date_time), 0, (struct sockaddr *) &server_address, &server_address_length)) == -1) {
                    perror("Error [Could not receive from the server]: ");
                    exit(EXIT_FAILURE);
                } else {
                    received = 1;
                    printf("Current date and time is given below: \n\n%s\n", date_time);
                } break;
        }

        if (received) {
            break;
        }
 
        if (tries == 5) {

            /*******************************\
            | Close connection after 5 tries |
            \*******************************/
            printf("\nThe server did not send any data. Timeout exceeded!\n");
            break;
        }
    }

    /*****************\
    | Close the socket |
    \*****************/          
    close(sockfd); 

    return 0; 
} 