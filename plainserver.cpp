#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024 //Define network data buffer size

int main(){
    int server_socket, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address); 
    char buffer[BUFFER_SIZE] = {0}; //Temp storage for 'chat' data

    //Prompt for server port
    int port;
    std::cout << "Enter the port number the server will listen on: ";
    std::cin >> port;
    std::cin.ignore(); //Clear the newline character from the buffer

    //Create server socket
    if((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Sever Socket Failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET; //IPv4
    address.sin_addr.s_addr = INADDR_ANY; //Bind to local device NIC IP address
    address.sin_port = htons(port); 

    //Bind socket to port
    if(bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0){
        perror("Socket-Port Bind Failed");
        exit(EXIT_FAILURE);
    }

    //Listen for incoming connections
    if(listen(server_socket, 3) < 0){
        perror("Socket Listen Failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << port << "...\n";
    std::cout << "Ready for plaintext communication.\n";

    //Accept incoming connection
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);
    if((client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_addrlen)) < 0){
        perror("Accept Incoming Connection Failed");
        exit(EXIT_FAILURE);
    }

    //Connection successful
    std::cout << "Incoming connection from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port);

    //**Set up 'two-way' chat**
    //Prompt for server name
    std::string server_name;
    std::cout << "\nEnter your username: ";
    std::getline(std::cin, server_name);

    //Send server name to client
    send(client_socket, server_name.c_str(), server_name.length(), 0);

    //Receive client name
    int len = recv(client_socket, buffer, BUFFER_SIZE, 0);
    buffer[len] = '\0';
    std::string client_name(buffer);
    std::cout << client_name << " has joined the chat.";

    //Recieve and send 'chat' data
    while(true){
        //Server Message
        std::string message;
            std::cout << server_name << ": ";
            std::getline(std::cin, message);

        //Exit chat if server user types 'exitchat'
        if(message == "exitchat"){
            send(client_socket, message.c_str(), message.length(), 0);//Inform client chat ended
            std::cout << server_name << " ended chat. ";
            break;
        }

        //Send client message
        send(client_socket, message.c_str(), message.length(), 0); 

        //Receive Client Message
        len = recv(client_socket, buffer, BUFFER_SIZE, 0);
        buffer[len] = '\0';

        //If client exited the chat
        if(std::string(buffer) == "exitchat"){
            std::cout << client_name << " exited the chat. ";
            break;
        }

        //Else display client message
        std::cout << client_name << ": " << buffer << std::endl;
    }

    //Clean up - Close Sockets
    close(client_socket);
    close(server_socket);
    return 0;
}