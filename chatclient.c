/*******************************************************************************************
 * Author:		Keisha Arnold
 * Filename: 	chatclient.c
 * Description: A simple client-server chat application utilizing the sockets API.
 *              This is the client program, it connects to a server program at an address
 *              specified by the first CL argument and a port # specified by the second
 *              CL argument. Then the program connects to the server and the two hosts
 *              take turns chatting. The client sends the first message and either host 
 *		        can terminate the connection with "\quit".
 ********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define HANDLE_LEN 10
#define MSG_LEN	500

/*******************************************************************************************
 * Function: 		void error(const char *msg)
 * Description:		Error function used for reporting issues
 * Parameters:		char *msg, message printed to stderr
 * Pre-Conditions: 	msg must be initialized
 * Post-Conditions: msg prints to stderr and program exits with status 0
 ********************************************************************************************/
void error(const char *msg) {
	perror(msg);
	exit(0);
}

/*********************************************************************************************
 * Function: 		struct sockaddr_in serverSocketInit(char* hostName, char* portNum)
 * Description:		Sets up the server socket sockaddr_in struct
 * Parameters:      char* hostName is argv[1], the server's host name
 *                  char* portNum is argv[2], the server's port #
 * Pre-Conditions: 	The server must be running at the specified socket, listening for
 *			        connections
 * Post-Conditions: returns an initialized sockaddr_in struct
 **********************************************************************************************/
struct sockaddr_in serverSocketInit(char* hostName, char* portNum) {
	int portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(portNum);			// Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname(hostName);	// Convert the machine name into a special form of address
	if (serverHostInfo == NULL) {
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	// Copy in the address
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
	
	return serverAddress;
}

/*********************************************************************************************
 * Function: 		int connectSocket(struct sockaddr_in serverAddress)
 * Description:		Sets up the server socket file descriptor and connect the client
 *			        to the server
 * Parameters:		struct sockaddr_in, the server's address
 * Pre-Conditions: 	The server must be running at the specified socket socket,
 *                  the struct sockaddr_in must be properly initialized
 * Post-Conditions: The client and server successfully connect or it throws an error and exits
 **********************************************************************************************/
int connectSocket(struct sockaddr_in serverAddress) {
	int socketFD;
	
	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) {
		error("CLIENT: ERROR opening socket");
	}
	
	// Connect to server
	// Connect socket to address
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) { 		
		error("CLIENT: ERROR connecting");
	}
	return socketFD;
}

/*******************************************************************************************
 * Function: 		char* getUserHandle(char* userHandle)
 * Description:		Function to get a handle from the user
 * Parameters:		char* userHandle
 * Pre-Conditions: 	char* userHandle declared in main
 * Post-Conditions: Returns an initialized userHandle specified by the user on the CL
 ********************************************************************************************/
char* getUserHandle(char* userHandle) {
	memset(userHandle, '\0', sizeof(*userHandle)); // Clear out the buffer array
	printf("Enter a handle up to 10 characters, no spaces: ");
	scanf("%s", userHandle);
	//printf("User Handle: %s \n", userHandle);
	
	// validate user handle
	while(userHandle[10] != 0) {
		//printf("userhandle[10]: %s\n", &userHandle[10]);
		printf("User handle not valid.\nPlease enter a handle up to 10 characters, no spaces: ");
		memset(userHandle, '\0', strlen(userHandle));
		scanf("%s", userHandle);
	}
	
	return userHandle;
}

/***********************************************************************************************
 * Function: 		int initiateContact (int socketFD, char* userHandle, char* serverHandle)
 * Description:		Mimics a TCP handshake. Client sends their userHandle and server responds
 *			        with their user handle. Handles will be displayed on CL throughout the chat.
 * Parameters:		int socketFD, the socket file descriptor
			        char* userHandle, the client's handle
			        char* serverHandle, the server's handle
 * Pre-Conditions: 	The server must be running at the specified socket,
 *                 	userHandle initialized with user specified handle,
			        serverHandle declared in main
 * Post-Conditions: The server receives the userHandle, the client receives the serverHandle,
                    the serverHandle is initialized with the server's response,
			        TCP handshake complete.
 ************************************************************************************************/
int initiateContact(int socketFD, char* userHandle, char* serverHandle) {
	int charsWritten;
	int charsRecv;
	
	// Send userHandle to server
	charsWritten = send(socketFD, userHandle, strlen(userHandle), 0); // Write to the server
	
	while (charsWritten < 0) {
		charsWritten = send(socketFD, userHandle, strlen(userHandle), 0);
	}
	
	// Get return message from server
	memset(serverHandle, '\0', sizeof(*serverHandle)); // Clear out the buffer
	// Read data from the socket, leaving \0 at end
	charsRecv = recv(socketFD, serverHandle, HANDLE_LEN, 0);
	while (charsRecv < 0) {
		charsRecv = recv(socketFD, serverHandle, HANDLE_LEN, 0);
	}
	//printf("CLIENT: I received this from the server: %s\n", serverHandle);
	printf("serverHandle: %s\n", serverHandle);
	return 0;
	
}

/***********************************************************************************************
 * Function: 		int sendMessage(int socketFD, char* userHandle, , char* serverHandle)
 * Description:		Function to send messages to the server through the connected socket
 *			        until the client closes the connection with "\quit".
 * Parameters:		int socketFD, char* userHandle, , char* serverHandle
 * Pre-Conditions: 	The server must be running at the specified socket,
 *                  userHandle initialized with user specified handle,
 *			        serverHandle initialized with user specified handle,
 *			        the server and client are connected with a dedicated socket.
 * Post-Conditions: The client sends a message to the server, or closes the connection.
 ************************************************************************************************/
int sendMessage(int socketFD, char* userHandle, char* serverHandle) {
	char clientBuffer[MSG_LEN + 1] = {0};
	memset(clientBuffer, '\0', sizeof(clientBuffer));
	int charsWritten;

	printf("%s> ", userHandle);
	
	while (strlen(clientBuffer) < 1) {
		memset(clientBuffer, '\0', sizeof(clientBuffer));
		fgets(clientBuffer, sizeof(clientBuffer), stdin);
		clientBuffer[strcspn(clientBuffer, "\n")] = 0;
	}
	
	// if user inputs "\quit"
	if(strcmp(clientBuffer, "\\quit") == 0) {
		printf("Closing the connection with %s\n", serverHandle);
		return 1;
	}

	// send message to server
	charsWritten = send(socketFD, clientBuffer, strlen(clientBuffer), 0); // Write to the server
	while (charsWritten < 0) {
		charsWritten = send(socketFD, userHandle, strlen(userHandle), 0);
	}
	return 0;
}

/***********************************************************************************************
 * Function: 		int recvMessage(int socketFD, char* userHandle, , char* serverHandle)
 * Description:		Function to receive messages from the server through the connected socket
 *			        until the server closes the connection with "\quit".
 * Parameters:		int socketFD, char* userHandle, , char* serverHandle
 * Pre-Conditions: 	The server must be running at the specified socket,
 *                  userHandle initialized with user specified handle,
 *			        serverHandle initialized with user specified handle,
 *			        the server and client are connected with a dedicated socket.
 * Post-Conditions: The client receives a message from the server, or the server closes
 *			        the connection.
 ************************************************************************************************/
int recvMessage(int socketFD, char* userHandle, char* serverHandle) {
	
	char serverBuffer[MSG_LEN + 1];
	int charsRecv;

	// Get return message from server
	memset(serverBuffer, '\0', sizeof(serverBuffer)); // Clear out the buffer
	// Read data from the socket, leaving \0 at end
	charsRecv = recv(socketFD, serverBuffer, MSG_LEN, 0);
	while (charsRecv < 0) {
		charsRecv = recv(socketFD, serverBuffer, MSG_LEN, 0);
	}
	
	if(charsRecv == 0) {
		printf("Connection closed by %s.\n", serverHandle);
		return 1;
	}
	
	printf("%s> %s\n", serverHandle, serverBuffer);
	return 0;
}


/*******************************************************************************************
 * Function: 		int main(int argc, char *argv[])
 * Description:		main function of the client gets CL arguments, sets up server address,
 *			        connects to the socket, gets the user's handle, completes the TCP
 *			        handshake, send messages and receives messages from the server, closes
 *			        the connection when done chatting.
 * Parameters:		argv[0] is the program filename
 *                  argv[1] is the server's hostName (localhost)
 *                  argv[2] is the server's port #
 * Pre-Conditions: 	The server must be running at the specified socket
 * Post-Conditions: The two hosts exchange messages, when the client or server quits the
 *			        socket is closed and the program terminates.
 ********************************************************************************************/
int main(int argc, char *argv[]) {
	int socketFD;
	struct sockaddr_in serverAddress;
	char userHandle[HANDLE_LEN + 1];
	char serverHandle[HANDLE_LEN + 1];
	int clientConnect = 0;
	int serverConnect = 0;

	// Check usage & args
	if (argc < 3) {
		fprintf(stderr,"USAGE: %s <hostname> <port number>\n", argv[0]);
		exit(0);
	}

	// Set up server address struct
	serverAddress = serverSocketInit(argv[1], argv[2]);

	// Set up the socket
	socketFD = connectSocket (serverAddress);

	// Get handle from user
	memset(userHandle, '\0', sizeof(userHandle)); // Clear out the buffer array
	getUserHandle(userHandle);
	printf("User Handle: %s \n", userHandle);
 
	// Initiate Contact by sending handle
	memset(serverHandle, '\0', sizeof(serverHandle));
	initiateContact(socketFD, userHandle, serverHandle);
	printf("Handshake complete. \n");
	
	fflush(stdin);
	fflush(stdout);
	fseek(stdin,0,SEEK_END);

	// Chat with server
	while(1) {
		// Send message to server
		clientConnect = sendMessage(socketFD, userHandle, serverHandle);
		if(clientConnect == 1) { // client entered "\quit"
			break;
		}
		// Receive message from server
		serverConnect = recvMessage(socketFD, userHandle, serverHandle);
		if(serverConnect == 1) { // server entered "\quit"
			break;
		}
	}
	
	// Close the socket
	close(socketFD);
	printf("Connection closed. Goodbye!\n");
 
	return 0;
}
