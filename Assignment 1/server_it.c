#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>

/****************************************\
|                STACK                    |
| N   : Maximum size of the stack         |
| top : Current size of the stack         |                               
| data: Array of strings storing the data |
\****************************************/
typedef struct _Stack {
    int N;
    int top;
    char ** data; 
} Stack;

/*****************************************\
| Function to create a stack of given size |
\*****************************************/
Stack * createStack(int N) {
    Stack * stack = (Stack *) malloc(sizeof(Stack));
    stack->N = N;
    stack->top = -1;
    stack->data = (char **) malloc(N * sizeof(char *));
    for (int i = 0; i < N; i++) {
        stack->data[i] = NULL;
    } return stack;
}

/******************************\
| Function to push onto a stack |
\******************************/
void push(Stack * stack, char * s) {
    stack->top++;
    stack->data[stack->top] = s; 
}

/*****************************\
| Function to pop from a stack |
\*****************************/
void pop(Stack * stack) {
    stack->data[stack->top] = NULL;
    stack->top--;
}

/******************************\
| Function to get the string at |
| the top of the stack          |
\******************************/
char * top(Stack * stack) {
    return stack->data[stack->top];
}

/******************************\
| Function to check whether the |
| the stack is empty or not     |
\******************************/
int isEmpty(Stack * stack) {
    return stack->top == -1;
}

/****************************\
| Function to reverse a stack |
\****************************/
Stack * reverse(Stack * stack) {
    Stack * newStack = createStack(stack->N);
    while (!isEmpty(stack)) {
        push(newStack, top(stack));
        pop(stack);
    } return newStack;
}

/*********************************\
| Function to check if a character |
| is a digit                       |
\*********************************/
int isDigit(char c) {
    return (c >= '0' && c <= '9');
}

/*********************************\
| Function to check if a character |
| is a digit or a decimal point    |
\*********************************/
int isDigitOrDot(char c) {
    return (isDigit(c) || c == '.');
}

/*********************************\
| Function to check if a character |
| is an operator or parentheses    |
\*********************************/
int isOpOrParentheses(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')');
}

/**************************************\
| Function to get a token from a source |
| given tail and head pointers          |
\**************************************/
char * getToken(char * s, int tail, int head) {
    // Ignore spaces before and after
    while (s[tail] == ' ') tail++;
    while (s[head] == ' ') head--;

    // Consturct and return the token if exists, otherwise return NULL
    if (tail <= head) {
        char * token = (char *) malloc((head - tail + 1) * sizeof(char));
        for (int i = tail; i <= head; i++) {
            token[i - tail] = s[i];
        } return token; 
    } return NULL;
}
 
/*********************************************\
| Function to evaluate a given arithmetic      |
| expression and return the result as a string |
\*********************************************/
char * evaluate(char * s) {
    // Count the number of tokens in the expression
    int token_count = 0;
    int state = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (state == 0) {
            if (isDigitOrDot(s[i])) {
                state = 1;
                token_count++;
            } else if (isOpOrParentheses(s[i])) {
                token_count++;
            }
        } else {
            if (s[i] == ' ') {
                state = 0;
            } else if (isOpOrParentheses(s[i])) {
                state = 0;
                token_count++;
            }
        }
    }

    // Extract all the tokens and store them in an array of strings
    char ** tokens = (char **) malloc(token_count * sizeof(char *));
    int count = 0;
    state = 0;
    int i = 0, pi = 0;
    for (i = 0; s[i] != '\0'; i++) {
        if (state == 0) {
            if (isDigitOrDot(s[i])) {
                state = 1;
                char * token = getToken(s, pi, i - 1);
                if (token) {
                    tokens[count] = token;
                    count++;
                    pi = i;
                } 
            } else if (isOpOrParentheses(s[i])) {
                char * token = getToken(s, pi, i - 1);
                if (token) {
                    tokens[count] = token;
                    count++;
                    pi = i;
                } 
            }
        } else {
            if (s[i] == ' ') {
                state = 0;
            } else if (isOpOrParentheses(s[i])) {
                state = 0;
                char * token = getToken(s, pi, i - 1);
                if (token) {
                    tokens[count] = token;
                    count++;
                    pi = i;
                }
            }
        }
    }

    char * token = getToken(s, pi, i - 1);
    if (token) {
        tokens[count] = token;
        count++;
        pi = i;
    }

    // Use a stack to evaluate the expression
    Stack * stack = createStack(token_count);
    for (int i = 0; i < token_count; i++) {
        if (isDigit(tokens[i][0])) {
            if (!isEmpty(stack) && isOpOrParentheses(top(stack)[0]) && top(stack)[0] != '(') {
                char op = *top(stack);
                pop(stack);
                double firstOperand = atof(top(stack));
                pop(stack);
                double secondOperand = atof(tokens[i]);
                double res;
                
                switch (op) {
                    case '+': 
                            res = firstOperand + secondOperand;
                            break;
                    case '-':
                            res = firstOperand - secondOperand;
                            break;
                    case '*': 
                            res = firstOperand * secondOperand;
                            break;
                    case '/': 
                            res = firstOperand / secondOperand;
                            break;
                }

                char * newToken = (char *) malloc(100 * sizeof(char));   

                gcvt(res, 8, newToken);
                push(stack, newToken);
            } else {
                push(stack, tokens[i]);
            }
        } else if (tokens[i][0] == ')') {
            char * operand = top(stack);
            pop(stack);
            if (top(stack)[0] == '(') {
                pop(stack);
                push(stack, operand);
            } else {
                if (!isEmpty(stack) && isOpOrParentheses(top(stack)[0]) && top(stack)[0] != '(') {
                    char op = *top(stack);
                    pop(stack);
                    double firstOperand = atof(top(stack));
                    pop(stack);
                    double secondOperand = atof(operand);
                    double res;

                    switch (op) {
                        case '+': 
                                res = firstOperand + secondOperand;
                                break;
                        case '-':
                                res = firstOperand - secondOperand;
                                break;
                        case '*': 
                                res = firstOperand * secondOperand;
                                break;
                        case '/': 
                                res = firstOperand / secondOperand;
                                break;
                    }

                    char * newToken = (char *) malloc(100 * sizeof(char));  

                    gcvt(res, 8, newToken);
                    push(stack, newToken);
                }
            }
            
        } else {
            push(stack, tokens[i]);
        }
    }

    stack = reverse(stack);
    while (stack->top > 1) {
        double firstOperand = atof(top(stack));
        pop(stack);
        char op = *top(stack);
        pop(stack);
        double secondOperand = atof(top(stack));
        pop(stack);

        double res;
        switch (op) {
            case '+': 
                    res = firstOperand + secondOperand;
                    break;
            case '-':
                    res = firstOperand - secondOperand;
                    break;
            case '*': 
                    res = firstOperand * secondOperand;
                    break;
            case '/': 
                    res = firstOperand / secondOperand;
                    break;
        }

        char * newToken = (char *) malloc(100 * sizeof(char));  

        gcvt(res, 8, newToken);
        push(stack, newToken);
    }

    return top(stack);
}

/*********************************************\
| Function to receive an arithmetic expression |
| from the client in byte sized chunks         |
| Returns: 0 on success, -1 on failure         |
\*********************************************/
int receiveAll(int clientSocket, char ** expressionAddr) {
    int bytesRead = 0;
    char charReceived = ' ', * buf = NULL;
    int n = 0;

    do {
        n = recv(clientSocket, &charReceived, 1, 0);
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
    // The server socket and the client socket
	int	serverSocket, clientSocket; 

    // The server address and the client address
	struct sockaddr_in clientAddress, serverAddress;

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
		perror("Error: Unable to bind local address!\n");
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

        // Receive the '\0' terminated arithmetic expression from the client
        char * expression = NULL;
        receiveAll(clientSocket, &expression);

        // Compute the value of the arithmetic expression obtained from the client and send back the result
        char * res = evaluate(expression);
        printf("Successfully computed the value of the arithmetic expression.\n");
        send(clientSocket, res, strlen(res) + 1, 0);

        // Close the client socket
        close(clientSocket);
	}

	return 0;
}