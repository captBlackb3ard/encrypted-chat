#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFFER_SIZE 1024

void init_openssl(){
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl(){
    EVP_cleanup();
}

//Create TLS/SSL Context
SSL_CTX* create_context(){
    const SSL_METHOD* method = SSLv23_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if(!ctx){
        perror("Unable to create SSL context");
         ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

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

    init_openssl();
    SSL_CTX* ctx = create_context();

    //Create client socket
    if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Client Socket Failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    //Convert Server IP to binary
    if(inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0){
        perror("Invalid server IPv4 address or address not supported.");
        exit(EXIT_FAILURE);
    }

    //Connect to server
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Server Connection Failed");
        exit(EXIT_FAILURE);
    }

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_socket);

    if(SSL_connect(ssl) <= 0){
        ERR_print_errors_fp(stderr);
    } else {
        //**Set up 'two-way' chat**

        // Receive server username
        int len = SSL_read(ssl, buffer, BUFFER_SIZE);
        buffer[len] = '\0';
        std::string server_name(buffer);
        std::cout << "Connected to " << server_name << "\n";

        //Prompt client name
        std::string client_name;
        std::cout << "\nEnter your username: ";
        std::getline(std::cin, client_name);
        SSL_write(ssl, client_name.c_str(), client_name.length());

        while (true) {
            // Receive message from server
            len = SSL_read(ssl, buffer, BUFFER_SIZE);
            buffer[len] = '\0';

            // If server has exited chat
            if (std::string(buffer) == "exitchat") {
                std::cout << server_name << " has ended the chat.\n";
                break;
            }

            // Display server message with prefix
            std::cout << server_name << ": " << buffer << std::endl;

            // Send a message
            std::cout << client_name << ": ";
            std::string message;
            std::getline(std::cin, message);

            // Exit chat if client user types 'exitchat'
            if (message == "exitchat") {
                SSL_write(ssl, message.c_str(), message.length());
                std::cout << "Chat ended by client.\n";
                break;
            }

            // Send message to server
            SSL_write(ssl, message.c_str(), message.length());
        }

    }


    //Clean up - Close Sockets & TLS Connection
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_socket);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 0;
}