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
    while(ss >> arg) args.push_back(arg);

    if (command == "join") {
        handleJoin(args);
    } else if(command == "exit") {
        handleExit(args);
    }
    else if(command == "logout") {
        handleLogout(args);
    }
    // ... other commands ...
}

void ProcessorInput::handleJoin(const std::vector<std::string>& args) {
    if(!isValidJoinCommand(args)) {
        std::cout << "Invalid join command. Usage: join {game_name}" << std::endl;
        return;
    }
    std::string gameName = args[0];
    
    // 1. UPDATE PROTOCOL STATE
    int subId = protocol.generateSubId();
    protocol.addSubscription(gameName, subId);
    
    
   // 1. Build the frame content (Headers + Body)
    std::string receiptId = std::to_string(protocol.generateReceiptId());
    std::string frame = "SUBSCRIBE\n";
    frame += "destination:/" + gameName + "\n";
    frame += "id:" + std::to_string(subId) + "\n";
    frame += "receipt:" + receiptId + "\n";
    frame += "\n"; // The empty line indicating end of headers

    // 2. SEND FRAME
    // We use sendBytes because we need to send the Null Byte manually
    // and we don't want sendLine adding extra newlines.

    bool success = true;

    // A. Send the string content
    if (!connection.sendBytes(frame.c_str(), frame.length())) {
        success = false;
    }

    // B. Send the null character explicitly
    if (success && !connection.sendBytes("\0", 1)) {
        success = false;
    }

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }
    if(!protocol.getShouldTerminate()){
        protocol.addReceiptAction(std::stoi(receiptId), "SUBSCRIBE" + gameName);
        protocol.addSubscription(gameName,subId);
    }
        
}
bool ProcessorInput::isValidJoinCommand(const std::vector<std::string>& args) {
    // A valid join command has exactly one argument: the game name
    return args.size() == 1 ;
}

void ProcessorInput::handleExit(const std::vector<std::string>& args){
    if(!isValidExitCommand(args)){
        std::cout << "Invalid exit command. Usage: exit {game_name}" << std::endl;
        return;
    }
    int subId=protocol.getSubscriptionId(args[0]);
    std::string receiptId = std::to_string(protocol.generateReceiptId());
    std::string frame="UNSUBSCRIBE\n";
    frame+="id:"+subId+'\n';
    frame+="receipt:"+receiptId+'\n'+'\n';
    
    bool success=true;
    // A. Send the string content
    if (!connection.sendBytes(frame.c_str(), frame.length())) {
        success = false;
    }

    // B. Send the null character explicitly
    if (success && !connection.sendBytes("\0", 1)) {
        success = false;
    }

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }
    if(!protocol.getShouldTerminate()){
        protocol.addReceiptAction(std::stoi(receiptId), "UNSUBSCRIBE" + args[0]);
        protocol.removeSubscription(args[0]);
    }
}
bool ProcessorInput::isValidExitCommand(const std::vector<std::string>& args){
    return args.size() == 1 && !protocol.isSubscribedTo(args[0]);
}

void ProcessorInput::handleLogout(const std::vector<std::string>& args){
    if(!isValidLogoutCommand(args)){
        std::cout << "Invalid logout command. Usage: logout " << std::endl;
        return;
    }
    std::string receiptId = std::to_string(protocol.generateReceiptId());
    std::string frame="DISCONNECT\n";
    frame+="receipt:"+receiptId+'\n'+'\n';
    
    bool success=true;
    // A. Send the string content
    if (!connection.sendBytes(frame.c_str(), frame.length())) {
        success = false;
    }

    // B. Send the null character explicitly
    if (success && !connection.sendBytes("\0", 1)) {
        success = false;
    }

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }
    if(!protocol.getShouldTerminate()){
        protocol.addReceiptAction(std::stoi(receiptId), "DISCONNECT");
    }
}

bool ProcessorInput::isValidLogoutCommand(const std::vector<std::string>& args){
    return args.size()==0;
}



