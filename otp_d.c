#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>

pid_t processes[5] = {-5,-5,-5,-5,-5};

void killChildProcesses(int signo){
	int childExitStatus = -5;
	int i;
	for(i=0; i < 5; i++){
		kill(processes[i], SIGINT);
	}
	exit(0);
}

void intHandlerChild(int signo){
	char* message = "Stopping Child\n";
    write(STDOUT_FILENO, message, 15);
	exit(0);
}




void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues


int main(int argc, char *argv[])
{
	signal(SIGINT, killChildProcesses);
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[100000];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	pid_t spawnpid;
	int i = 0;
	for(i = 0; i < 5; i++){
		spawnpid = processes[i] = fork();
		if(processes[i] == 0)
			break;
		else if(processes[i] == -1){
			printf("Hull Breach! \n");
			fflush(stdout);
			exit(1);break;
		}
	}
	switch(spawnpid){
		case 0:
			while(1){
				signal(SIGINT, intHandlerChild);
				sleep(2);
				sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
				establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
				if (establishedConnectionFD < 0) error("ERROR on accept");

				// Get the message from the client and display it
				memset(buffer, '\0', 256);
				charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer), 0); // Read the client's message from the socket
				if (charsRead < 0) error("ERROR reading from socket");
				printf("SERVER Process %d: I received this from the client: \"%s\"\n",i, buffer);

				// Send a Success message back to the client
				charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
				if (charsRead < 0) error("ERROR writing to socket");
				close(establishedConnectionFD); // Close the existing socket which is connected to the client
			}
		default:
			while(1){
				
			}
		}
	close(listenSocketFD); // Close the listening socket
	return 0; 
}
