#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <netinet/in.h>

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

/******************************************************************\
| Function to find whether a given username exists in a file or not |
| Parameters: file_name - The name of the file                      |
|             username  - The username                              |
| Returns: 0 - false, 1 - true                                      |
\******************************************************************/
int username_exists(const char * file_name, const char * username) {
    char current_username[50];
    FILE * fstream = fopen(file_name, "r");
    while (!feof(fstream)) {
        fscanf(fstream, "%s", current_username);
        if (!strcmp(current_username, username)) {
            return 1;
        }
    } return 0;
}

/*******************************************************\
| Function to check the name of a command with arguments |
| Parameters: command - The command with arguments       |
|             buffer  - The name of the command          |
| Returns: 0 - false, 1 - true                           |
\*******************************************************/
int begins_with(const char * command, const char * command_name) {
    int len = strlen(command_name);
    if (len <= strlen(command)) {
        for (int i = 0; i < len; i++) {
            if (command[i] != command_name[i]) {
                return 0;
            }
        } return 1;
    } return 0;
}

/**************************************************\
| Function to format a command for easier handling  |
| Parameters: command - The command to be formatted |
| Returns: void, the command is modified in place   |
\**************************************************/
void format_command(char * command) {
    int spaces = 0;
    char formatted_command[MAX_BUFFER_SIZE];
    memset(formatted_command, '\0', sizeof(formatted_command));
    int i = 0, j = 0;
    for (; command[i] == ' '; i++);
    for (; command[i] != ' ' && command[i] != '\0'; i++, j++) formatted_command[j] = command[i];
    for (; command[i] == ' '; i++);
    if (command[i] != '\0') {
        formatted_command[j] = ' '; j++;
    }
    for (; command[i] != '\0'; i++, j++) formatted_command[j] = command[i];
    formatted_command[j] = '\0';

    strcpy(command, formatted_command);
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
    
    /************\
    | The sockets |
    \************/
	int	sockfd, newsockfd; 

    /******************************************\
    | The server address and the client address |
    \******************************************/
	struct sockaddr_in server_address, client_address;

    /**************************\
    | The client address length |
    \**************************/
    socklen_t client_address_length;

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
	server_address.sin_family	   = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port		   = htons(7575);

    /**************************************\
    | Bind the socket to the server address |
    \**************************************/
	if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		perror("Error [Unable to bind local address]");
		exit(EXIT_FAILURE);
	}

    /************************************************************\
    | Start the server which can handle upto 5 concurrent clients |
    \************************************************************/
	listen(sockfd, 5);
	printf("\nServer Running....\n");

    while (1) {

        /************************************************************************\
        | Accept an incoming client connection request and get the client details |
        \************************************************************************/
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

            /****************************************\
            | Send the "LOGIN: " prompt to the client |
            \****************************************/
            memset(buffer, '\0', sizeof(buffer));
            strcpy(buffer, "LOGIN: \0");
            send_all(newsockfd, buffer, strlen(buffer) + 1, 50);

            /***********************************\
            | Receive the username of the client |
            \***********************************/
            memset(buffer, '\0', sizeof(buffer));
            receive_all(newsockfd, buffer,  50); 
            
            if (username_exists("users.txt", buffer)) {

                /***********************************************************\
                | If username exists, send the message "FOUND" to the client |
                \***********************************************************/
                memset(buffer, '\0', sizeof(buffer));
                strcpy(buffer, "FOUND\0");
                send_all(newsockfd, buffer, strlen(buffer) + 1, 50);

                while (1) {

                    /**************************************************\
                    | Receive the command from the client and format it |
                    \**************************************************/
                    memset(buffer, '\0', sizeof(buffer));
                    receive_all(newsockfd, buffer, 50);
                    format_command(buffer);

                    /********************************\
                    | Handle each command accordingly |
                    \********************************/
                    if (!strcmp(buffer, "pwd\0")) {

                        memset(buffer, '\0', sizeof(buffer));
                        if (getcwd(buffer, sizeof(buffer))) {

                            /******************************************************\
                            | Send the current working directory name to the client |
                            \******************************************************/
                            send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
                        } else {

                            /***************************************************************\
                            | Send to the client that the command execution was unsuccessful |
                            \***************************************************************/
                            memset(buffer, '\0', sizeof(buffer));
                            strcpy(buffer, "####\0");
                            send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
                        }
                    } else if (begins_with(buffer, "dir")) {

                        DIR * dirp = NULL;

                        /*******************************************\
                        | Default argument for "dir" command is "./" |
                        \*******************************************/
                        if (strlen(buffer) <= 4) {
                            dirp = opendir("./");
                        } else {
                            dirp = opendir(buffer + 4);
                        }

                        struct dirent * direntp = NULL;
                        char * dir_entries = NULL;
                        memset(buffer, '\0', sizeof(buffer));

                        if (dirp != NULL) {

                            /************************************************************************\
                            | Get all the file names in the given directory and send it to the client |
                            \************************************************************************/
                            while ((direntp = readdir(dirp)) != NULL) {
                                strcat(buffer, direntp->d_name);
                                strcat(buffer, "\n");
                            } closedir(dirp);
                            strcat(buffer, "\0");
                            send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
                        } else {

                            /***************************************************************\
                            | Send to the client that the command execution was unsuccessful |
                            \***************************************************************/
                            memset(buffer, '\0', sizeof(buffer));
                            strcpy(buffer, "####\0");
                            send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
                        }
                    } else if (begins_with(buffer, "cd")) {

                        if (strlen(buffer) > 3 && chdir(buffer + 3) == 0) {

                            /****************************************************************************\
                            | Send current working directory after running the "cd" command to the client |
                            \****************************************************************************/
                            send_all(newsockfd, buffer + 3, strlen(buffer + 3) + 1, 50);
                        } else {

                            /***************************************************************\
                            | Send to the client that the command execution was unsuccessful |
                            \***************************************************************/
                            memset(buffer, '\0', sizeof(buffer));
                            strcpy(buffer, "####\0");
                            send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
                        }
                    } else if (!strcmp(buffer, "exit\0")) {

                        /***********************************************\
                        | Close the client connection for "exit" command |
                        \***********************************************/
                        break;
                    } else {

                        /**********************************************************\
                        | Send to the client that the command execution was invalid |
                        \**********************************************************/
                        memset(buffer, '\0', sizeof(buffer));
                        strcpy(buffer, "$$$$\0");
                        send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
                    }
                }
            } else {

                /***********************************************************************\
                | If username does not exist, send the message "NOT-FOUND" to the client |
                \***********************************************************************/
                memset(buffer, '\0', sizeof(buffer));
                strcpy(buffer, "NOT-FOUND\0");
                send_all(newsockfd, buffer, strlen(buffer) + 1, 50);
            }

            /************************************************\
            | Close the new socket and exit the child process |
            \************************************************/
            close(newsockfd);
            exit(0);   
        }

        /*****************\
        | Close the socket |
        \*****************/
        close(newsockfd);
    }

    return 0;
}