# client-server-chat-app

To run chatserve.py:
	Go to the directory containing the source files

	Enter the following on the command line:
	./chatserve.py <port#>

To run chatclient.c:
	Go to the directory containing the source files

	To compile enter the following on the command line:
	make

	To run enter the following on the command line:
	./chatclient localhost <port#>


Instructions:
	Each program will have you enter a handle. The handle can be up to 10 characters
	and must not have spaces. chatclient will send their handle as the initial message
	and server will respond with their handle.  This is the "handshake". After
	that they will take turns sending messages until one of them enters "\quit".
	Note that it will not send an empty message. If you hit enter without typing
	anything on the command line, it will simply go to the next line and wait for
	input. The server will keep it's serverSocket open and listen for new connections
	until a SIGINT is received.  

