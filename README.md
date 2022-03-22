# Chat Room System.

The project manages a chat room for discussion for many users simultaneously.


For this project we have two components:
- Client
- Server

# Server:

 The server manages the conexions and will manage the communication among the clients connected.
The clients connect on the server each having an user id, given by the server and a unique name, which they choose when connecting to the server.
The server receives the messages from the clients and immediatly sends them back to the others clients in the chat room.
The server has 3 types of messaging functions:
 - From client to all clients: used when somebody sends a message 
 - From server to all clients: used when somebody joins/ leaves the room 
 - From server to a client: used when an error is met (e.g. "username is already taken")

# Client:

 The client connects to the server and after entering his name he will have a text interface which he will use to send a message.
The message sent by a client will be sent simultaneously to the other clients logged.

In this project we have use C programming language.
 We have used threads, signals, memory manipulation functions, mutexes.

The code is implemented in such way that we check for overflows and other errors on both client and server side.
The idea behind this is that if something fails on the client side, it will not be sent to the server so we don't overload the server, but we also check on 
the server side in case someone modifies the client code.

# Other ideas:

- CommandChecker
- Private messages
- Custom commands, for now only !exit is implemented
    Some other useful commands: !whisper (private message), !help, !Vote-kick.
- Word censorship
- ChatRoom password
- Saving server logs: messages, IPs, time stamps etc.
    
    
# How to run the program:

> gcc -pthread -o client.o client.c && ./client.o -> to run the client code

> gcc -pthread -o superServer.o superServer.c && ./superServer.o -> to run the server code.

The server needs to be started first, otherwise when a client connents an error message will be sent.

:)
