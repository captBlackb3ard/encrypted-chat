#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024 //Define network data buffer size

int main(){
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};

    std::string server_ip;
    int server_port;

    //Prompt client for server IP and Port
    std::cout << "Enter server IP address: ";
    std::cin >> server_ip;
    std::cout << "Enter server port: ";
    std::cin >> server_port;
    std::cin.ignore();

    //Create client socket
    if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Client Socket Failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    //Conver Server IP to binary
    if(inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0){
        perror("Invalid server IPv4 address or address not supported.");
        exit(EXIT_FAILURE);
    }

    //Connect to server
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Server Connection Failed");
        close (client_socket);
        exit(EXIT_FAILURE);
    }

    //**Set up 'two-way' chat**
    //Receive server name
    int len = recv(client_socket, buffer, BUFFER_SIZE, 0);
    buffer[len] = '\0';
    std::string server_name(buffer);
    std::cout << server_name << " has joined the chat";

    //Prompt client name
    std::string client_name;
    std::cout << "\nEnter your username: ";
    std::getline(std::cin, client_name);

    //Send client name to server
    send(client_socket, client_name.c_str(), client_name.length(), 0);

    //Recieve and send 'chat' data
    while(true){
        //Prompt for client message
        std::string message;
        std::cout << client_name << ": ";
        std::getline(std::cin, message);

       // Exit chat if client types 'exitchat'
       if(message == "exitchat"){
            send(client_socket, message.c_str(), message.length(), 0);
            std::cout << client_name <<" ended chat.";
            break;
        }

        //Else send message to server
        send(client_socket, message.c_str(), message.length(), 0);
        
        //Recieve message from server
        len = recv(client_socket, buffer, BUFFER_SIZE, 0);
        buffer[len] = '\0';

        //If client exited chat
        if(std::string(buffer) == "exitchat"){
            std::cout << server_name << " exited the chat.";
            break;
        }

        //Else display server messages
        std::cout << server_name << ": " << buffer << std::endl;
    }

    //Clean up - Close Sockets
    close(client_socket);
    return 0;
}