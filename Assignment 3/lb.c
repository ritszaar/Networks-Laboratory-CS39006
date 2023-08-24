#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <time.h>
#include <sys/poll.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

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

/*****************************************\
| Function to get ip address from a socket |                         
| Parameters: socketfd - The socket        |                         
| Returns: The ip address                  |   
\*****************************************/
char * get_ip(int sockfd) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(sockfd, (struct sockaddr *) &addr, &addr_size);
    char * ip = (char *) malloc(20 * sizeof(char));
    strcpy(ip, inet_ntoa(addr.sin_addr));
    return ip;    
}
  
int main(int argc, char ** argv) {

    /*****************\
    | The port numbers |
    \*****************/
    int balancer_port;
    int server_ports[2];
    if (argc < 4) {
        printf("Error [Unable to get port numbers]: Run as <executable file> <own port> <S1's port> <S2's port>.\n");
        exit(EXIT_FAILURE);
    } else {
        balancer_port   = atoi(argv[1]);
        server_ports[0] = atoi(argv[2]);
        server_ports[1] = atoi(argv[3]);
    }

    /************\
    | The sockets |
    \************/
    int newsockfd, sockfd;
    int server_sockfd[2]; 

    /**************\
    | The addresses |
    \**************/
    struct sockaddr_in server_addresses[2]; 
    struct sockaddr_in balancer_address, client_address;

    /**************************\
    | The client address length |
    \**************************/
    socklen_t client_address_length;

    /****************************\
    | Define the server addresses |
    \****************************/
    for (int i = 0; i < 2; i++) {
        server_addresses[i].sin_family = AF_INET; 
        server_addresses[i].sin_port = htons(server_ports[i]); 
        inet_aton("127.0.0.1", &server_addresses[i].sin_addr);
    }

    /*****************************************\
    | Create the socket for client connections |
    \*****************************************/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        perror("Error [Could not create socket]"); 
        exit(EXIT_FAILURE); 
    }

    /****************************\
    | Define the balancer address |
    \****************************/
    balancer_address.sin_family = AF_INET;
    balancer_address.sin_addr.s_addr = INADDR_ANY;
    balancer_address.sin_port = htons(balancer_port);

    /****************************************\
    | Bind the socket to the balancer address |
    \****************************************/
	if (bind(sockfd, (struct sockaddr *) &balancer_address, sizeof(balancer_address)) < 0) {
		perror("Error [Unable to bind local address]");
		exit(EXIT_FAILURE);
	}

    /***************************************\
    | Start listening for client connections |
    \***************************************/
    listen(sockfd, 5);

    /**********************************\
    | The buffer used for communication |
    \**********************************/
    char buffer[500];

    /************************\
    | The load on the servers |
    \************************/
    int loads[2];
    loads[0] = loads[1] = 0;

    /*********************************\
    | Define the variables for polling |
    \*********************************/
    struct pollfd fd;
    int poll_result, received = 0;
    fd.fd = sockfd;
    fd.events = POLLIN;

    clock_t prev_time, cur_time;
    double wait_time = 5000;
    prev_time = clock();

    while (1) {
        poll_result = poll(&fd, 1, wait_time);

        switch (poll_result) {
            case -1:

                /**********************\
                | Exit if polling fails |
                \**********************/
                perror("Error [Polling unsuccessful]");
                exit(EXIT_FAILURE);

            case 0:

                /***********************************\
                | Get server loads if timeout occurs |
                \***********************************/
                printf("\n");
                for (int i = 0; i < 2; i++) {
                    memset(buffer, '\0', sizeof(buffer));
                    strcpy(buffer, "Send Load\0");

                    /************************************\
                    | Create socket and connect to server |
                    \************************************/
                    if ((server_sockfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
                        perror("Error [Could not create socket]"); 
                        exit(EXIT_FAILURE); 
                    }

                    if ((connect(server_sockfd[i], (struct sockaddr *) &server_addresses[i], sizeof(server_addresses[i]))) < 0) {
		                perror("Error [Unable to connect to server]");
		                exit(EXIT_FAILURE);
	                }

                    /********************\
                    | Get the server load |
                    \********************/
                    send_all(server_sockfd[i], buffer, strlen(buffer) + 1, 50);
                    receive_all(server_sockfd[i], buffer, sizeof(buffer));
                    loads[i] = atoi(buffer);
                    printf("Load received from server at %s port %d: %d\n", get_ip(server_sockfd[i]), server_ports[i], loads[i]);

                    /*****************\
                    | Close the socket |
                    \*****************/
                    close(server_sockfd[i]);
                }

                prev_time = clock();
                wait_time = 5000;
                break;

            case 1:

                if ((newsockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_address_length)) < 0) {
                    perror("Error [Unable to accept client request]");
                    exit(EXIT_FAILURE);
                } 

                /******************************************************\
                | Start a child process for handling the current client |
                \******************************************************/
                if (fork() == 0) {

                    /*********************\
                    | Close the old socket |
                    \*********************/
                    close(sockfd);
                    
                    memset(buffer, '\0', sizeof(buffer));
                    receive_all(newsockfd, buffer, 50);

                    if (!strcmp(buffer, "Send Time\0")) {
                        int less_loaded = loads[0] < loads[1] ? 0 : 1;
                        printf("\nSending client request to server at 127.0.0.1, port %d.\n", server_ports[less_loaded]);
                        
                        /************************************\
                        | Create socket and connect to server |
                        \************************************/
                        if ((server_sockfd[less_loaded] = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
                            perror("Error [Could not create socket]"); 
                            exit(EXIT_FAILURE); 
                        }

                        if ((connect(server_sockfd[less_loaded], (struct sockaddr *) &server_addresses[less_loaded], sizeof(server_addresses[less_loaded]))) < 0) {
		                    perror("Error [Unable to connect to server]");
		                    exit(EXIT_FAILURE);
	                    }

                        /*************************************\
                        | Get the current time from the server |
                        \*************************************/
                        send_all(server_sockfd[less_loaded], buffer, strlen(buffer) + 1, 50);
                        receive_all(server_sockfd[less_loaded], buffer, 50);

                        /*****************\
                        | Close the socket |
                        \*****************/
                        close(server_sockfd[less_loaded]);

                        /************************************\
                        | Send the current time to the client |
                        \************************************/
                        send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
                    }

                    /************************************************\
                    | Close the new socket and exit the child process |
                    \************************************************/
                    close(newsockfd);
                    exit(0); 
                }
                
                /**********************************************************\
                | Adjust wait time according to time taken by child process |
                \**********************************************************/
                cur_time = clock();
                wait_time = wait_time - ((cur_time - prev_time) / (double) CLOCKS_PER_SEC * 1000);
                break;
            default:
                break;
        }
    }

    return 0; 
} 
