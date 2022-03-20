# Chat Room System.

The project manages o chat room for discussion for many users simultaneously.


For this project we have two components:
- Client
- Server

# Server:

 The server manages the conexions and will manage the communication among the clients connected.
The clients connect on the server each having an user id, given by the server and an unique name, which they choose when connecting to the server.
The server receives the messages from the clients and immediatly sents it back to the others clients in the chat room.

# Client:

 The clients connects to the server and after entering its name he will be having a text interface which will be used to sent the message he types.
The message sent by a client will be sent simultaneously to the other clients logged.

In this project we coded in the C programming language.
 We have used threads, signals, memory manipulation functions, mutexes.

The code is implemented in such way that we check for overflows and other errors on both client and server side.
The idea behind this is that if something fails on the client side, it will not be sent to the server so we don't overload the server, but we also check on 
the server side in case someone modifies the client code.

# Other ides:

- CommandChecker
- Private messages
- Custom commands, for now only !exit is implemented
    Some other useful commands: !whisper (private message), !help, !Vote-kick.
- Word censorship
- ChatRoom password
    
    
# How to run the program:

gcc -pthread -o client.o client.c && ./client.o -> to run the client code
gcc -pthread -o superServer.o superServer.c && ./superServer.o -> to run the server code.

The server needs to be started first, otherwise when a client connents an error message will be sent.

