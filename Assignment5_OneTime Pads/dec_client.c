#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  
#include <sys/socket.h> 
#include <netdb.h> 

// Reference: From Assignment 5 pages, under Hints

#define MAXCHAR 100000

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Take input from files (encrypted text and key) and send it as a message to the server.
* 3. Print the decrypted message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
    perror(msg); 
    exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname) {
 
    // Clear out the address struct
    memset((char*) address, '\0', sizeof(*address)); 

    // The address should be network capable
    address->sin_family = AF_INET;
    // Store the port number
    address->sin_port = htons(portNumber);

    // Get the DNS entry for this host name
    struct hostent* hostInfo = gethostbyname(hostname); 
    if (hostInfo == NULL) { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(0); 
    }
    // Copy the first IP address from the DNS entry to sin_addr.s_addr
    memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

// Main Function
int main(int argc, char* argv[]) {
    
    int socketFD, portNumber, charsWritten, charsRead, bytesSent;
    struct sockaddr_in serverAddress;
    char buffer[MAXCHAR*2];
    char smallBuff[MAXCHAR];
    
    // Check usage & args
    if (argc < 3) { 
        fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
        exit(0); 
    } 

    FILE *keyFile;
    FILE *textFile;
    char line[MAXCHAR];
    char key[MAXCHAR];
    int i;

// Open and read key file
    keyFile = fopen(argv[2], "r");
    if (keyFile == NULL) {
        error("ERROR Failed to open key file");
    }

    fgets(key, MAXCHAR, keyFile);

    int keyLen = strlen(key);
    if (key[keyLen - 1] == '\n') {
        key[keyLen - 1] = ' '; // Replace the newline character from fgets with a space
    }

    fclose(keyFile);

    // Open and read text file
    textFile = fopen(argv[1], "r");
    if (textFile == NULL) {
        error("ERROR Failed to open text file");
    }

    fgets(line, MAXCHAR, textFile);
    int lineLen = strlen(line);

    if (keyLen < lineLen) {
        error("ERROR Key is too short"); // If key is shorter than the message
    }

    if (line[lineLen - 1] == '\n') { // Replace new line with space if exist
        line[lineLen - 1] = ' ';
    }

    for (i = 0; i < lineLen; i++) { //loop
        int asciiValue = line[i];

        if (!((asciiValue >= 'A' && asciiValue <= 'Z') || asciiValue == ' ')) { //check for ALP or Space
            error("ERROR Bad character found");
        }
    }

    fclose(textFile);


    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0) {
        error("CLIENT: ERROR opening socket");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        error("CLIENT: ERROR connecting");
    }

    // Clear out the buffer array
    memset(buffer, '\0', sizeof(buffer));
    
    char decIdentifier[] = "dec,";
    sprintf(buffer, "%s%s%s%s%s", decIdentifier, line, ",", key, "\n"); // Move all parts of the message into the buffer
    
    // Send message to server
    // Write to the server
    charsWritten = 0;
    bytesSent = 0;
    int messageLength = strlen(buffer);

    while (charsWritten < messageLength) {
        bytesSent = send(socketFD, buffer, messageLength, 0); 
        if (bytesSent < 0) {
            error("CLIENT: ERROR writing to socket");
            exit(1);
        }
        charsWritten += bytesSent;
    }

    // Get return message from server
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end

    int bytesReceived = 0;
    charsRead = 0;

    while (charsRead < lineLen) {
        bytesReceived = recv(socketFD, smallBuff, sizeof(smallBuff) - 1, 0); // leave space for null terminator

        if (bytesReceived < 0) {
            error("CLIENT: Error reading from socket");
            break;
        } else if (bytesReceived == 0) {
            // Done reading
            break;
        }

        smallBuff[bytesReceived] = '\0'; // null-terminate the received data
        strncat(buffer, smallBuff, bytesReceived);

        charsRead += bytesReceived;
    }


    buffer[strlen(buffer)-1] = '\n';
    printf("%s", buffer);

    // Close the socket
    close(socketFD); 
    return 0;
}