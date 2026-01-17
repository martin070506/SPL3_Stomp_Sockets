#include "../include/ProcessorOutput.h"
#include "../include/event.h"
#include <iostream>
#include <sstream>
#include <vector>

ProcessorOutput::ProcessorOutput(StompProtocol& protocol,ConnectionHandler& connection)
    : protocol(protocol),connection(connection) {}

void ProcessorOutput::run() {
    ConnectionHandler* connection = protocol.getConnection();

    if (!connection) {
        std::cerr << "Error: Listener thread started without an active connection!" << std::endl;
        protocol.setShouldTerminate(true);
        return;
    }

    while (!protocol.getShouldTerminate()) {
        std::string frame;
        if (!connection->getFrameAscii(frame, '\0')) {
            std::cout << "Disconnected from server (Socket closed)" << std::endl;
            protocol.setShouldTerminate(true);
            break;
        }

        process(frame);
    }
}

void ProcessorOutput::process(const std::string& frame) {
    std::cout << "Frame Gotten:\n" << frame <<std::endl;
    std::stringstream ss(frame);
    std::string command;
    
    std::getline(ss, command);

    std::string line;
    std::map<std::string, std::string> headers;
    
    while (std::getline(ss, line) && line != "") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            headers[key] = value;
        }
    }

    std::string body;
    char c;
    while (ss.get(c)) 
        body += c;
    
    if (command == "CONNECTED") 
        handleConnected(headers);
    if (command == "MESSAGE") 
        handleMessage(headers, body);
    if (command == "RECEIPT") 
        handleReceipt(headers);
    if (command == "ERROR") 
        handleError(headers, body);
}

void ProcessorOutput::handleConnected(const std::map<std::string, std::string>& headers) {
    std::cout << "Login successful" << std::endl;
    protocol.setConnected(true);
}

void ProcessorOutput::handleMessage(const std::map<std::string, std::string>& headers, const std::string& body) {
    std::string user = "";
    std::stringstream ss(body);
    std::string line;
    // Read the first line, which usually contains "user: name"
    if (std::getline(ss, line)) {
        if (line.find("user: ") == 0) {
            user = line.substr(6); // Skip "user: " (6 chars)
        }
        else if (line.find("user:") == 0) {
             user = line.substr(5); // Skip "user:" (5 chars) handle case without space
        }
    }

    Event event(body);

    protocol.addEvent(event, user);

    std::cout << "Game update received from " << user << ": " 
              << event.get_name() << std::endl;
}

void ProcessorOutput::handleReceipt(const std::map<std::string, std::string>& headers) {

    if (headers.count("receipt-id")) 
        try {
            int receiptId = std::stoi(headers.at("receipt-id"));
            protocol.processReceipt(receiptId);
        } catch (const std::exception& e) {
            std::cout << "Error parsing receipt-id" << std::endl;
        }

}

void ProcessorOutput::handleError(const std::map<std::string, std::string>& headers, const std::string& body) {
    // std::cout << "Error from server: " << std::endl;
    // if (headers.count("message")) 
    //     std::cout << "Message: " << headers.at("message") << std::endl;
    
    std::cout << body << std::endl;
    
    protocol.setShouldTerminate(true);

    if (protocol.getConnection()) 
        protocol.getConnection()->close();
}