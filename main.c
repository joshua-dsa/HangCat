#include <stdio.h> // Include standard input/output library
#include <stdlib.h> // Include standard library
#include <string.h> // Include string manipulation library
#include <winsock2.h> // Include Windows Sockets API library for networking. Yes, this code is made to run on windows
#include <time.h> // Include time library for generating random numbers
#include <process.h> // Include process library for multi-threading to handle multiple clients simultaneously

// #pragma comment(lib, "ws2_32.lib") // This is a pragma directive to link the ws2_32.lib library.

#define PORT 8080 // Define a constant named PORT with the value 8080, which is the port the server will run on.
#define MAX_BUFFER_SIZE 1024 // Define a constant named MAX_BUFFER_SIZE with the value 1024, which is the maximum amount of data to be received by the server.

// Function to send a response to the user upon a query
void send_response(SOCKET client_socket, const char *response) {
    // Store the result of sending data to the client socket in a variable
    int result = send(client_socket, response, strlen(response), 0);
    // Check for errors in the result
    if (result == SOCKET_ERROR) {
        perror("send"); // Print an error message if there's an issue with sending data
    }
}

// This function is used to send files to the user
void send_file_response(SOCKET client_socket, const char *file_path) {
    FILE *file = fopen(file_path, "rb"); // Open the file specified by file_path in binary read mode
    if (file == NULL) {
        // If the requested file path does not exist, return a 404 Not Found error
        send_response(client_socket, "HTTP/1.1 404 Not Found\r\n\r\n");
        return;
    }

    fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
    fseek(file, 0, SEEK_SET); // Move the file pointer back to the beginning of the file

    char response[MAX_BUFFER_SIZE * 2]; // Create a character buffer for the response, increased buffer size
    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n\r\n"); // Create an HTTP response header
    send_response(client_socket, response); // Send the HTTP response header to the client

    char file_buffer[MAX_BUFFER_SIZE]; // Create a buffer to read and send the file data
    size_t bytes_read;
    while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
        if (send(client_socket, file_buffer, bytes_read, 0) == SOCKET_ERROR) {
            perror("send"); // Print an error message if there's an issue with sending file data
            break;
        }
    }

    fclose(file); // Close the file
}

// Read the words.txt file and get a random word from the file index.
void handle_randomword_request(SOCKET client_socket) {

    // Open the words.txt file in the binary format
    FILE *file = fopen("words.txt", "r");

    // If the file does not exist, return a 500 Internal Server Error
    if (file == NULL) {
        send_response(client_socket, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
        return;
    }

    char words[MAX_BUFFER_SIZE]; 
    size_t word_count = 0;
    while (fgets(words, sizeof(words), file) != NULL) {
        word_count++;
    }
    fclose(file);

    srand(time(NULL)); // Seed the random number generator with the current time
    int random_index = rand() % (int)word_count; // Generate a random index within the word count

    file = fopen("words.txt", "r"); // Reopen the file
    for (size_t i = 0; i <= (size_t)random_index; i++) { 
        fgets(words, sizeof(words), file); // Read lines from the file until the random index is reached
    }
    fclose(file);

    char response[MAX_BUFFER_SIZE * 2]; // Create a character buffer for the response, increased buffer size
    snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n\r\n%s", words); // Create an HTTP response with the random word
    send_response(client_socket, response); // Send the HTTP response to the client
}

void handle_request(SOCKET client_socket) {
    char buffer[MAX_BUFFER_SIZE]; // Create a buffer to receive data from the client
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0); // Receive data from the client
    if (bytes_received == SOCKET_ERROR) {
        perror("recv"); // Print an error message if there's an issue with receiving data
        closesocket(client_socket); // Close the client socket
        return;
    }
    if (bytes_received == 0) {
        closesocket(client_socket); // Close the client socket if no data was received
        return;
    }

    printf("Received request: %s\n", buffer); // Print the received request to the console

    if (strstr(buffer, "GET /randomword") != NULL) {
        handle_randomword_request(client_socket); // Handle a request for a random word
    } else if (strstr(buffer, "GET / ") != NULL || strstr(buffer, "GET /index.html") != NULL) {
        send_file_response(client_socket, "public/index.html"); // Handle a request for the index.html file
    } else if (strstr(buffer, "GET /script.js") != NULL) {
        send_file_response(client_socket, "public/script.js"); // Handle a request for the script.js file
    } else if (strstr(buffer, "GET /style.css") != NULL) {
        send_file_response(client_socket, "public/style.css"); // Handle a request for the style.css file
    } else if (strstr(buffer, "GET /Puss_in_Boots.glb") != NULL) {
        send_file_response(client_socket, "public/Puss_in_Boots.glb"); // Handle a request for the Puss_in_Boots.glb file
    } else {
        send_response(client_socket, "HTTP/1.1 404 Not Found\r\n\r\n"); // Handle a request for an unknown resource
    }

    closesocket(client_socket); // Close the client socket
}

void client_handler(void *client_socket_ptr) {
    SOCKET client_socket = *((SOCKET *)client_socket_ptr); // Get the client socket from the pointer
    handle_request(client_socket); // Handle the client's request
    closesocket(client_socket); // Close the client socket
    _endthread(); // End the thread
}

int main() {
    WSADATA wsa; // Structure for Windows Sockets API initialization
    SOCKET listen_socket, client_socket; // Declare socket variables for listening and client connections
    struct sockaddr_in server, client; // Declare socket address structures for server and client

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        perror("WSAStartup failed"); // Initialize Windows Sockets API, print an error message if it fails
        return 1;
    }

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        perror("Socket creation failed"); // Create a socket for listening, print an error message if it fails
        WSACleanup(); // Clean up Windows Sockets API
        return 1;
    }

    server.sin_family = AF_INET; // Set the server socket's address family
    server.sin_addr.s_addr = INADDR_ANY; // Set the server socket's IP address to any available network interface
    server.sin_port = htons(PORT); // Set the server socket's port number

    if (bind(listen_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        perror("Bind failed"); // Bind the server socket to the specified address and port, print an error message if it fails
        closesocket(listen_socket); // Close the listening socket
        WSACleanup(); // Clean up Windows Sockets API
        return 1;
    }

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        perror("Listen failed"); // Start listening for client connections, print an error message if it fails
        closesocket(listen_socket); // Close the listening socket
        WSACleanup(); // Clean up Windows Sockets API
        return 1;
    }

    printf("Server listening on port %d...\n", PORT); // Print a message indicating the server is listening on the specified port

    while (1) {
        int client_size = sizeof(client); // Get the size of the client socket address structure
        client_socket = accept(listen_socket, (struct sockaddr *)&client, &client_size); // Accept a client connection
        if (client_socket == INVALID_SOCKET) {
            perror("Accept failed"); // Print an error message if accepting the client connection fails
            continue;
        }

        _beginthread(client_handler, 0, &client_socket); // Create a new thread to handle the client
    }

    closesocket(listen_socket); // Close the listening socket
    WSACleanup(); // Clean up Windows Sockets API

    return 0; // Return 0 to indicate successful program execution
}