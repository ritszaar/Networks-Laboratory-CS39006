#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

/*************************************************\
| Function to receive the result of the arithmetic |
| expression from the sever in byte sized chunks   |
| Returns: 0 on success, -1 on failure             |
\*************************************************/
int receiveAll(int serverSocket, char ** expressionAddr) {
    int bytesRead = 0;
    char charReceived = ' ', * buf = NULL;
    int n = 0;

    do {
        n = recv(serverSocket, &charReceived, 1, 0);
        if (n == -1) { 
            break; 
        }
        bytesRead++;
        buf = (char *) realloc(buf, bytesRead * sizeof(char));
        buf[bytesRead - 1] = charReceived;
    } while (charReceived != '\0');

    *expressionAddr = buf;
    return n; 
} 

int main() {
    // The client socket
	int clientSocket;

    // The server address
    struct sockaddr_in serverAddress;

    // Define the server address
    serverAddress.sin_family = AF_INET;
	inet_aton("127.0.0.1", &serverAddress.sin_addr);
	serverAddress.sin_port	= htons(20000);

    int choice = 0;
    while (choice != -1) {
        // Create the client socket
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Error: Unable to create client socket!\n");
            exit(0);
        }

        // Connect the client socket to the server address
        if ((connect(clientSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress))) < 0) {
		    printf("Error: Unable to connect to server!\n");
		    exit(0);
	    }

        // Read an arithmetic expression from the console
        printf("\nEnter an arithmetic expression: ");
        char * string = NULL;
        size_t size = 10;
        int bytesRead = getline(&string, &size, stdin);
        string[bytesRead] = '\0';

        // Send the expression entered to the server
        send(clientSocket, string, bytesRead + 1, 0);

        // Get the evaluated expression back from the server
        char * buf = NULL;
        receiveAll(clientSocket, &buf);
        double res = atof(buf);

        // Print the result onto the console
        printf("The value of the expression is: %lf\n", res);
        printf("\nDo you want to enter another expression? [No : -1, Yes: otherwise]: ");
        scanf("%d", &choice);
        getchar();

        // Close the client socket after evaluation
        close(clientSocket);
    }

    printf("\n");

    return 0;
}