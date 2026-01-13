#include "../include/ProcessorInput.h"
#include <sstream> 
#include <string>
#include <vector>
#include <iostream>

// Constructor
ProcessorInput::ProcessorInput() {
    // EMPTY
}

std::string ProcessorInput::ParseValueFromRequest(std::string& clientRequest, std::string& value){
    // Implementation here
    return ""; 
}

std::string ProcessorInput::ProcessConnectRequest(std::string& clientRequest){
    std::stringstream ss(clientRequest);
    std::string command,serverAddress,host,port,login,passcode,check;
    
    ss >> command >> serverAddress >> login >> passcode;
    std::stringstream ss(serverAddress);
    std::getline(ss, host, ':');
    std::getline(ss, port);

}

std::string ProcessorInput::ProcessSubscribeRequest(std::string& clientRequest){
    // Implementation
}

std::string ProcessorInput::ProcessUnsubscribeRequest(std::string& clientRequest){
    // Implementation
}

std::string ProcessorInput::ProcessSendRequest(std::string& clientRequest){
    // Implementation
}

std::string ProcessorInput::ProcessSummarizeRequest(std::string& clientRequest){
    // Implementation
}

std::string ProcessorInput::ProcessRequest(std::string &clientRequest){
    std::stringstream ss(clientRequest);
    std::string command;
    ss >> command;
    if(command == "login")
        ProcessConnectRequest(clientRequest);
}

bool isValidLoginCommand(const std::string& line) {
    std::stringstream ss(line);
    std::string segment;
    std::vector<std::string> args;

    // 1. Split by Space
    while(ss >> segment) {
        args.push_back(segment);
    }

    // CHECK 1: Must have exactly 4 parts (login, host:port, user, pass)
    if (args.size() != 4) {
        std::cout << "Error: Invalid number of arguments." << std::endl;
        return false;
    }

    // CHECK 2: First word must be "login"
    if (args[0] != "login") {
        std::cout << "Error: Command must start with 'login'." << std::endl;
        return false;
    }

    // CHECK 3: Second part must look like "host:port"
    std::string hostPort = args[1];
    std::size_t colonPos = hostPort.find(':');

    if (colonPos == std::string::npos) {
        std::cout << "Error: Host must be in format 'ip:port'." << std::endl;
        return false;
    }

    // CHECK 4: Verify the port is actually a number
    std::string portStr = hostPort.substr(colonPos + 1);
    try {
        int port = std::stoi(portStr);
        if (port <= 0 || port > 65535) {
             std::cout << "Error: Port number out of range." << std::endl;
             return false;
        }
    } catch (...) {
        std::cout << "Error: Port is not a valid number." << std::endl;
        return false;
    }

    return true;
}