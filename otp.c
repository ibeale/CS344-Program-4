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


// Encrypts plain text, takes in a buffer to store the encrypted plaintext, the plaintext, and a key
char* encryptPlaintext(char* buffer, char* plaintext, char* key){
    int lenPT = strlen(plaintext);
    int lenK = strlen(key);
    memset(buffer, '\0',11);
    if(lenPT > lenK){fprintf(stderr,"Plaintext is too long for the key provided.\n"); return NULL;}
    int i = 0;
    char letters[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    for(i;i<lenPT;i++){
        int idx =findIndex(letters, plaintext[i]);
        if(idx == -1){fprintf(stderr,"Plaintext contains uncompatible characters.\n"); return NULL;}
        else{
            char new_letter_idx = findIndex(letters, letters[((idx + findIndex(letters, key[i]))%27)]);
            printf("newidx: %d\n", new_letter_idx);
            // printf("%c -> %c\n", plaintext[i], key[new_letter_idx]);
            buffer[i] = letters[new_letter_idx];
        }
    }
    buffer[i] = '\0';
    printf("%s\n", buffer);
    return buffer;
}

char* unencryptCiphertext(char* buffer, char* ciphertext, char* key){
    int lenPT = strlen(ciphertext);
    int lenK = strlen(key);
    memset(buffer, '\0',11);
    if(lenPT > lenK){fprintf(stderr,"ciphertext is too long for the key provided.\n"); return NULL;}
    int i = 0;
    char letters[28] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
    for(i;i<lenPT;i++){
        int idx =findIndex(letters, ciphertext[i]);
        if(idx == -1){fprintf(stderr,"ciphertext contains uncompatible characters.\n"); return NULL;}
        else{
            int sum = idx - findIndex(letters, key[i]);
            char new_letter_idx = findIndex(letters, letters[((sum%27)+27)%27]); // custom mod function to work on negatives
            printf("newidx: %d\n", new_letter_idx);
            // printf("%c -> %c\n", ciphertext[i], key[new_letter_idx]);
            buffer[i] = letters[new_letter_idx];
        }
    }
    buffer[i] = '\0';
    printf("%s\n", buffer);
    return buffer;
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
	char plaintext[100000];
	char key[100000];
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


    memset(plaintext, '\0', sizeof(plaintext)); // Clear out the buffer array
    FILE* fp = fopen(argv[3], "r");

    char character = fgetc(fp); // Get input from the user, trunc to buffer - 1 chars, leaving \0
    int i=0;
    while(character != '\n' && character != EOF){
        plaintext[i] = character;
        character = fgetc(fp);
        i++;
    }
    // printf("%s\n",plaintext);
    fclose(fp);

    // Send message to server
    charsWritten = send(socketFD, plaintext, strlen(plaintext), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(plaintext)) printf("CLIENT: WARNING: Not all data written to socket!\n");

    // Get return message from server
    memset(plaintext, '\0', sizeof(plaintext)); // Clear out the buffer again for reuse
    charsRead = recv(socketFD, plaintext, sizeof(plaintext) - 1, 0); // Read data from the socket, leaving \0 at end
    if (charsRead < 0) error("CLIENT: ERROR reading from socket");
    printf("CLIENT: I received this from the server: \"%s\"\n", plaintext);

	close(socketFD); // Close the socket
	return 0;
}