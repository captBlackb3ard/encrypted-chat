#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
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
    const SSL_METHOD* method = SSLv23_server_method();
    SSL_CTX* ctx = SSL_CTX_new(method);
    if(!ctx){
        perror("Unable to create SSL context");
         ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

//Configure TLS/SSL Context with Server Certificate and Public Key
void configure_context(SSL_CTX* ctx){
    SSL_CTX_set_ecdh_auto(ctx, 1);
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0 || SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main(){
    int server_socket, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    init_openssl();
    SSL_CTX* ctx = create_context();
    configure_context(ctx);

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
    std::cout << "Ready for encrypted communication.\n";

    //Accept incoming connection
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);
    if((client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_addrlen)) < 0){
        perror("Accept Incoming Connection Failed");
        exit(EXIT_FAILURE);
    }

    //Connection successful
    std::cout << "Incoming connection from " << inet_ntoa(client_address.sin_addr) << ":" << ntohs(client_address.sin_port);

    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_socket);

    if(SSL_accept(ssl) <= 0){
        ERR_print_errors_fp(stderr);
    } else {
         //**Set up 'two-way' chat**

        //Prompt for server name
        std::string server_name;
        std::cout << "\nEnter your username: ";
        std::getline(std::cin, server_name);

        // Send server username to client
        SSL_write(ssl, server_name.c_str(), server_name.length());

        // Receive client username
        int len = SSL_read(ssl, buffer, BUFFER_SIZE);
        buffer[len] = '\0';
        std::string client_name(buffer);
        std::cout << client_name << " has joined the chat.\n";
        
        //Recieve and send 'chat' data
        while (true) {
            std::string message;
            std::cout << server_name << ": ";
            std::getline(std::cin, message);

            // Exit chat if server user types 'exitchat'
            if (message == "exitchat") {
                SSL_write(ssl, message.c_str(), message.length());
                std::cout << "Chat ended by server.\n";
                break;
            }

            // Send message to client
            SSL_write(ssl, message.c_str(), message.length());

            // Receive message from client
            len = SSL_read(ssl, buffer, BUFFER_SIZE);
            buffer[len] = '\0';

            // If client has exited chat
            if (std::string(buffer) == "exitchat") {
                std::cout << client_name << " has exited the chat.\n";
                break;
            }

            // Display client message with username
            std::cout << client_name << ": " << buffer << std::endl;
        }

    }


    //Clean up - Close Sockets & TLS Connection
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_socket);
    close(server_socket);
    SSL_CTX_free(ctx);
    cleanup_openssl();
    return 0;
}