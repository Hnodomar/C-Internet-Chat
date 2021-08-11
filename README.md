# Internet Chat Server/Client in C++

This project implements the Boost ASIO networking library to allow clients to chat to others over TCP and interact with a full suite of chat tools as part of a fully functioning chat application.

## Client Commands

All commands are entered by the user pre-pended with a '/' character.

* **Nick:** allows user to change their name if there is no other similarly named user in current chatroom.

* **List:** retrieves a list of currently available chatrooms from the server.

* **Users:** retrieves a list of users currently connected to the chatroom.

* **Create:** creates a new chatroom of the supplied name.

* **Join:** joins a chatroom of the supplied name if it exists.

## Code Features

* A basic application protocol has been designed in order to allow the server and client to interpret packets correctly. Packets have a header length of 3 bytes, consisting a 16-bit number that specifies the length of the body and a 1-byte long 'tag' that specifies how the packet is to be interpreted by the other end.

* Both server and client use Boost's asynchronous read/write calls to sockets.

* The client runs Boost ASIO's (asynchronous functionality) on a separate thread to the client interface.

* The server features a logging functionality that allows an administrator to track usage/traffic across channels, and can be toggled between standard output and a logging file with flag '-log' on server start from the CLI.

## Dependencies

* Boost 1.75 - set of C++ libraries; ASIO library used for networking https://www.boost.org/