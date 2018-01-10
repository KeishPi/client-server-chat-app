#!/usr/bin/env python

#****************************************************************************************#
# Author:		Keisha Arnold
# Filename:		chatserve.py
# Description:	A simple client-server chat application utilizing the sockets API.
#				This is the server program, it connects to a client application at a
#				port # specified by the second CL argument.
#				The server application is run first and gets the user's handle,
#				then sets up and binds the socket and begins listening for clients.
#				It then accepts the client's connection, does a TCP handshake by exchanging
#				handles with the client, and chats with the client until either host
#				terminates the connection with "\quit" or until a SIGINT is received.
# Sources :		Signal Handler- https://stackoverflow.com/questions/1112343/how-do-i-capture-sigint-in-python
#****************************************************************************************#
from socket import *
import sys #module w argv
import fcntl
import string
import signal


#****************************************************************************************#
# Function:			signal_handler(signal, frame)
# Description:		signal handler for SIGINT (Ctrl-c)
# Parameters:		signal, frame
# Pre-Conditions:	None
# Post-Conditions:	prints message to the screen and exits with code 0
#****************************************************************************************#
def signal_handler(signal, frame):
	print("SIGINT received, exiting chat server application")
	sys.exit(0)
signal.signal(signal.SIGINT, signal_handler)


#****************************************************************************************#
# Function:			getServerHandle()
# Description:		signal handler for SIGINT
# Parameters:		None
# Pre-Conditions:	None
# Post-Conditions:	Returns an initialized serverHandle specified by the user on the CL
#****************************************************************************************#
def getServerHandle():
	handle = raw_input("Enter a handle up to 10 characters, no spaces: ")
	while len(handle) == 0 or len(handle) > 10:
		handle = raw_input("Server handle not valid.\nPlease enter a handle up to 10 characters, no spaces: ")
	return str(handle).rstrip('\n')


#****************************************************************************************#
# Function:			recv(clientSocket, clientHandle, serverHandle)
# Description:		Function to receive messages from the client through the connected
#					socket until the client closes the connection with "\quit" or SIGINT
#					is received.
# Paramters:		clientSocket, clientHandle, serverHandle
# Pre-Conditions:	The server must be running at the specified socket,
#					clientHandle initialized, serverHandle initialized
# Post-Conditions:	The server receives a message from the server, or the client closes
#					the connection.
#****************************************************************************************#
def recv(clientSocket, clientHandle, serverHandle):
	clientMsg = clientSocket.recv(501)
	while (clientMsg <= 0):
		clientMsg = clientSocket.recv(501)
	clientMsg = str(clientMsg).rstrip('\n')
	#print "\{{}\} {}".format(clientHandle, clientMsg)
	if clientMsg == "":
		print("{} has terminated the chat connection. Disconnecting...".format(clientHandle))
		return 1
	print("{0}> {1}".format(clientHandle, clientMsg))


#****************************************************************************************#
# Function:			send(clientSocket, clientHandle, serverHandle)
# Description:		Function to send messages to the client through the connected
#					socket until the server closes the connection with "\quit" or SIGINT
#					is received.
# Paramters:		clientSocket, clientHandle, serverHandle
# Pre-Conditions:	The server must be running at the specified socket,
#					clientHandle initialized, serverHandle initialized
# Post-Conditions:	The server sends a message to the client, or the server closes
#					the connection.
#****************************************************************************************#
def send(clientSocket, clientHandle, serverHandle):
	serverMsg = raw_input("{0}> ".format(serverHandle))
	if serverMsg == "\quit":
		print ("You've terminated the chat with {}".format(clientHandle))
		return 1
	while serverMsg == "":
		serverMsg = raw_input("{0}> ".format(serverHandle))
	clientSocket.send(bytes(serverMsg))

#****************************************************************************************#
# Function:			handshake(clientSocket, serverHandle)
# Description:		Mimics a TCP handshake. Server receives client's handle and server
#					responds with their serverHandle. Handles will be displayed on CL
#					throughout the chat.
# Paramters:		clientSocket, serverHandle
# Pre-Conditions:	The server must be running at the specified socket,
#					serverHandle initialized
# Post-Conditions:	Server receives client's handle and server sends their handle,
#					the clientHandle is initialized with the client's response,
#					TCP handshake complete.
#****************************************************************************************#
def handshake(clientSocket, serverHandle):
	clientHandle = clientSocket.recv(10)
	print("Client Handle: {} ".format(clientHandle))
	clientSocket.send(bytes(serverHandle))
	return str(clientHandle).rstrip('\n')


#****************************************************************************************#
# check usage & args
numArgs = len(sys.argv)
if len(sys.argv) != 2:
	print ("USAGE: {} <server port #>\n".format(argv[0]))
	exit(1)

# get server handle
serverHandle = getServerHandle()
print("Server Handle: {} ".format(serverHandle))

# set up server socket
serverPort = sys.argv[1]  #create server socket at port specifed on command line
serverSocket = socket(AF_INET, SOCK_STREAM) #create TCP socket
serverSocket.bind(('', int(serverPort)))	#associate the port# (serverPort) with this socket
serverSocket.listen(1)	#listen for connection requests from the client, param specifies max # of queued connections
print ("The server is ready to receive on port {}".format(sys.argv[1]))

#listen for connections
while 1:
	#creates a new socket in the server dedicated to this particular client
	connectionSocket, addr = serverSocket.accept()
	print("Connection received from {}".format(addr))
	
	#get client's handle
	clientHandle = handshake(connectionSocket, serverHandle)
	print("Handshake complete.")
	
	#chat with client
	while 1:
		clientConnect = recv(connectionSocket, clientHandle, serverHandle)
		if clientConnect == 1:
			break
		serverConnect = send(connectionSocket, clientHandle, serverHandle)
		if serverConnect == 1:
			break
		
    #close this connection socket (but serverSocket still open for other clients)
	connectionSocket.close()
	print("Connection closed with client at socket: {}".format(addr))
	print("Listening for new connections...")


