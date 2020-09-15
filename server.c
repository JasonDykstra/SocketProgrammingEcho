#include <stdio.h>      // for printf() and fprintf()
#include <sys/socket.h> // for socket(), bind(), and connect()
#include <arpa/inet.h>  // for sockaddr_in and inet_ntoa()
#include <stdlib.h>     // for atoi() and exit()
#include <string.h>     // for memset()
#include <unistd.h>     // for close()

#define MAXPENDING 5    // Maximum outstanding connection requests
#define RCVBUFSIZE 32   // Size of receive buffer
void DieWithError(char *errorMessage){  // Error handling function
    printf("%s", errorMessage);
}
void HandleTCPClient(int clientSocket){ // TCP client handling function
    char buffer[RCVBUFSIZE];
    int numBytesReceived = recv(clientSocket, buffer, RCVBUFSIZE, 0);
    printf("Bytes received server side: %d", numBytesReceived);
    buffer[numBytesReceived] = '\0';
    if (numBytesReceived < 0) {
        DieWithError("recv() failed");
    }
    if(send(clientSocket, buffer, sizeof(buffer), 0) < 0){
        DieWithError("send() failed");
    }
}

int main(int argc, char *argv[]) {
    int serverSock;                     //Socket descriptor for server
    int clientSock;                     // Socket descriptor for client
    struct sockaddr_in echoServerAddr;  // Local address
    struct sockaddr_in echoClientAddr;  // Client address
    unsigned short echoServerPort;      // Server port
    unsigned int clientLen;             // Length of client address data structure

    if(argc != 2){                      // Test for correct number of argument
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServerPort = atoi(argv[1]);     // First arg: local port

    /* Create socket for incoming connections */
    if((serverSock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        DieWithError("socket() failed");
    }

    /* Construct local address structure */
    memset(&echoServerAddr, 0, sizeof(echoServerAddr)); // Zero out structure
    echoServerAddr.sin_family = AF_INET;                // Internet address family
    echoServerAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
    echoServerAddr.sin_port = htons(echoServerPort);    // Local port

    /* Bind to the local address */
    if(bind(serverSock, (struct sockaddr *) &echoServerAddr, sizeof(echoServerAddr)) < 0){
        DieWithError("bind() failed");
    }

    /* Mark the socket so it will listen for incoming connections */
    if(listen(serverSock, MAXPENDING) < 0){
        DieWithError("listen() failed");
    }

    while(1){  // Endless Loop
        /* Set the size of the in-out parameter */
        clientLen = sizeof(echoClientAddr); // Wait for a client to connect
        if ((clientSock = accept(serverSock, (struct sockaddr *) &echoClientAddr, &clientLen)) < 0) {
            DieWithError("accept() failed");
        }

        /* clientSock is connected to a client! */
        printf("Handling client %s\n", inet_ntoa(echoClientAddr.sin_addr));
        HandleTCPClient(clientSock);
        close(clientSock);
    }
}