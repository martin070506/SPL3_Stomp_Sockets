#include "../include/ProcessorInput.h"
#include "../include/StompProtocol.h"
#include "../include/ConnectionHandler.h"
#include <iostream>
#include <sstream>

ProcessorInput::ProcessorInput(StompProtocol& protocol,ConnectionHandler& connection): 
protocol(protocol), connection(connection){}

void ProcessorInput::process(const std::string& input) {
    std::stringstream ss(input);
    std::string command;
    ss >> command;
    
    std::vector<std::string> args;
    std::string arg;
    while (ss >> arg) 
        args.push_back(arg);

    if (command == "join") 
        handleJoin(args);
    if (command == "exit") 
        handleExit(args);
    if (command == "logout") 
        handleLogout(args);
}

void ProcessorInput::handleJoin(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Invalid join command. Usage: join {game_name}" << std::endl;
        return;
    }
    std::string gameName = args[0];
    
    int subId = protocol.generateSubId();
    protocol.addSubscription(gameName, subId);
    
    std::string receiptId = std::to_string(protocol.generateReceiptId());
    std::string frame = "SUBSCRIBE\n";
    frame += "destination:/" + gameName + "\n";
    frame += "id:" + std::to_string(subId) + "\n";
    frame += "receipt:" + receiptId + "\n";
    frame += "\n";

    bool success = true;

    if (!sendFrame(connection, frame)) 
        success = false;

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }

    if (!protocol.getShouldTerminate()) {
        protocol.addReceiptAction(std::stoi(receiptId), "SUBSCRIBE" + gameName);
        protocol.addSubscription(gameName,subId);
    }
}        


void ProcessorInput::handleExit(const std::vector<std::string>& args){
    if (args.size() != 1 || protocol.isSubscribedTo(args[0])) {
        std::cout << "Invalid exit command. Usage: exit {game_name}" << std::endl;
        return;
    }

    int subId = protocol.getSubscriptionId(args[0]);
    std::string receiptId = std::to_string(protocol.generateReceiptId());
    std::string frame = "UNSUBSCRIBE\n";
    frame += "id:" + subId + '\n';
    frame += "receipt:" + receiptId + "\n\n";
    
    bool success = true;
    if (!sendFrame(connection, frame))
        success = false;

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }
    if (!protocol.getShouldTerminate()) {
        protocol.addReceiptAction(std::stoi(receiptId), "UNSUBSCRIBE" + args[0]);
        protocol.removeSubscription(args[0]);
    }
}

void ProcessorInput::handleLogout(const std::vector<std::string>& args) {
    if (args.size() != 0) {
        std::cout << "Invalid logout command. Usage: logout " << std::endl;
        return;
    }

    std::string receiptId = std::to_string(protocol.generateReceiptId());
    std::string frame = "DISCONNECT\n";
    frame += "receipt:" + receiptId + "\n\n";
    
    bool success = true;
    if (!sendFrame(connection, frame))
        success = false;

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }
    if (!protocol.getShouldTerminate())
        protocol.addReceiptAction(std::stoi(receiptId), "DISCONNECT");
}

bool ProcessorInput::sendFrame(ConnectionHandler& connection, const std::string& frame) {
        return connection.sendBytes(frame.c_str(), frame.length()) &&
        connection.sendBytes("\0", 1);
}


