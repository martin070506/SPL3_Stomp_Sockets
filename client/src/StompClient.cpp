#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"

int main(int argc, char *argv[]) {
    
    ConnectionHandler* connection = nullptr;
    StompProtocol protocol=StompProtocol(*connection); // Assuming your protocol class doesn't need constructor args
    bool isLoggedIn = false;

    // --- PHASE 1: LOGIN LOOP ---
    while (!isLoggedIn) {
        const short bufsize = 1024;
        char buf[bufsize];
        
        std::cout << "Please login (login host:port username password):" << std::endl;
        std::cin.getline(buf, bufsize);
        std::string line(buf);
        
        // 1. Check if command is login
        std::stringstream ss(line);
        std::string command;
        ss >> command;
        
        if (command != "login") {
            std::cout << "You must log in first." << std::endl;
            continue;
        }

        // 2. Parse Host and Port manually (needed to create the socket)
        std::string hostPort;
        ss >> hostPort;
        size_t colonPos = hostPort.find(':');
        if (colonPos == std::string::npos) {
            std::cout << "Invalid host:port format" << std::endl;
            continue;
        }
        std::string host = hostPort.substr(0, colonPos);
        short port = (short) std::stoi(hostPort.substr(colonPos + 1));

        // 3. Create Connection and Connect
        connection = new ConnectionHandler(host, port);
        if (!connection->connect()) {
            std::cerr << "Could not connect to server " << host << ":" << port << std::endl;
            delete connection;
            connection = nullptr;
            continue;
        }

        // 4. Send the CONNECT frame
        // protocol.processInput() generates the frame and calls connection->sendLine(...)
        protocol.processInput(line); 

        // 5. BLOCKING WAIT for the "CONNECTED" frame
        // We must read the WHOLE frame (until \0) to clear the socket.
        std::string responseFrame;
        
        // Using the function you showed earlier to read until \0
        if (connection->getFrameAscii(responseFrame, '\0')) {
            
            // Check if the server said "CONNECTED"
            if (responseFrame.find("CONNECTED") != std::string::npos) {
                std::cout << "Login successful. Connected to server." << std::endl;
                isLoggedIn = true; 
            } else {
                std::cout << "Login failed. Server response:\n" << responseFrame << std::endl;
                std::cout << "Disconnecting..." << std::endl;
                connection->close();
                delete connection;
                connection = nullptr;
            }
        } else {
            std::cout << "Failed to receive handshake from server." << std::endl;
            connection->close();
            delete connection;
            connection = nullptr;
        }
    }

    // --- PHASE 2: GAME LOOP ---
    
    // 1. Start the Listener Thread
    // This thread will read all FUTURE messages (Receipts, Messages, Errors)
    // std::thread socketListener(readFromSocketTask, connection, &protocol);

    // 2. Run the Keyboard Loop (Main Thread)
    while (true) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);

        // Parse user input and send frames
        protocol.processInput(line, *connection);

        if (line == "logout") {
            // Note: The actual disconnection logic usually happens 
            // after receiving the RECEIPT for the DISCONNECT frame.
            // You might want a flag here to wait for that before breaking.
            break; 
        }
    }
    
    // socketListener.join(); // Wait for thread to finish if needed
    
    if (connection) {
        connection->close();
        delete connection;
    }

    return 0;
}