#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

// Takes a string and a letter, returns the index of the first instance of the letter.
int findIndex(char* arr, char letter){
    int len = strlen(arr);
    int i = 0;
    for(i;i<len; i++){
        if(arr[i] == letter){
            return i;
        }
    }
    return -1;
}

char* encryptPlaintext(char* buffer, char* plaintext, char* key){
    int lenPT = strlen(plaintext);
    int lenK = strlen(key);
    if(lenPT > lenK){fprintf(stderr,"Plaintext is too long for the key provided.\n"); return NULL}
    int i = 0;
    const char letters[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    for(i;i<lenPT;i++){
        int idx =findIndex(letters, plaintext[i]);
        if(idx == -1){fprintf(stderr,"Plaintext contains uncompatible characters.\n"); return NULL}
        else{
            char new_letter_idx = findIndex(letters, (idx + findIndex(letters, key[i])))
            buffer[i] = // This is where I left off. fill in buffer with letter[new_letter_idx]
        }
    }
}


// Checks the user input and assigns port number if input is good. Returns 1 if bad input
int checkUsage(int argc, char* argv[], int* portNumber){
    if (argc < 5) { fprintf(stderr,"USAGE: %s method user {plaintext} key port\n", argv[0]); return 1; } // Check usage & args
    if(strcmp(argv[1], "post")==0){
        if (argc != 6) { fprintf(stderr,"USAGE: %s post user plaintext key port\n", argv[0]); return 1; }
        *portNumber = atoi(argv[5]);
        return 0;
    }
    else if(strcmp(argv[1], "get")==0){
        if (argc != 5) { fprintf(stderr,"USAGE: %s get user key port\n", argv[0]); return 1; }
        *portNumber = atoi(argv[4]);
        return 0;
    }
    else { fprintf(stderr,"USAGE: %s method user {plaintext} key port\n", argv[0]); return 1; }
    
}

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[256];
    if(checkUsage(argc, argv, &portNumber)){exit(0);}; // If bad input, exit(0)

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
        error("CLIENT: ERROR connecting");

    // Get input message from user
    printf("CLIENT: Enter text to send to the server, and then hit enter: ");
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
    fgets(buffer, sizeof(buffer) - 1, stdin); // Get input from the user, trunc to buffer - 1 chars, leaving \0
    buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds

    // Send message to server
    charsWritten = send(socketFD, buffer, strlen(buffer), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");

    // Get return message from server
    memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

	close(socketFD); // Close the socket
	return 0;
}