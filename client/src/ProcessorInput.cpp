#include "../include/ProcessorInput.h"
#include "../include/StompProtocol.h"
#include "../include/ConnectionHandler.h"
#include <iostream>
#include <sstream>
#include <fstream>

ProcessorInput::ProcessorInput(StompProtocol& protocol, ConnectionHandler& connection) : 
    protocol(protocol), connection(connection) {}

void ProcessorInput::process(const std::string& input) {
    std::stringstream ss(input);
    std::string command;
    ss >> command;
    
    std::vector<std::string> args;
    std::string arg;
    while (ss >> arg) 
        args.push_back(arg);

    if (command == "join") handleJoin(args);
    else if (command == "exit") handleExit(args);
    else if (command == "logout") handleLogout(args);
    else if (command == "report") handleReport(args);
    else if (command == "summary") handleSummary(args);
    else if (command == "login") {
        std::cout << "The client is already logged in, log out before trying again" << std::endl;
    }
    else if (!protocol.getConnected()) {
        std::cout << "User Must Login first" << std::endl;
    }
    else {
        std::cout << "Invalid Command, Enter Something Valid" << std::endl;
    }
}

void ProcessorInput::handleJoin(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Invalid join command. Usage: join {game_name}" << std::endl;
        return;
    }
    std::string gameName = args[0];
    if (protocol.isSubscribedTo(gameName)) {
        std::cout << "You are already subscribed to " << gameName << std::endl;
        return; 
    }
    int subId = protocol.generateSubId();
    int receiptIdInt = protocol.generateReceiptId(); 
    std::string receiptId = std::to_string(receiptIdInt);

    std::string frame = "SUBSCRIBE\n";
    frame += "destination:/" + gameName + "\n";
    frame += "id:" + std::to_string(subId) + "\n";
    frame += "receipt:" + receiptId + "\n\n";

    protocol.addReceiptAction(receiptIdInt, "join " + gameName); 
    protocol.addSubscription(gameName, subId);


    if (protocol.getConnection()) {
        protocol.getConnection()->sendFrameAscii(frame, '\0');
    }
}   

void ProcessorInput::handleExit(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Invalid exit command. Usage: exit {game_name}" << std::endl;
        return;
    }
    std::string gameName = args[0];
    if (!protocol.isSubscribedTo(gameName)) {
        std::cout << "User is not subscribed to topic /" << gameName << std::endl; 
        return;
    }
    int subId = protocol.getSubscriptionId(gameName);
    int receiptIdInt = protocol.generateReceiptId();
    std::string receiptId = std::to_string(receiptIdInt);

    protocol.addReceiptAction(receiptIdInt, "exit " + gameName);
    protocol.removeSubscription(gameName);

    std::string frame = "UNSUBSCRIBE\n";
    frame += "id:" + std::to_string(subId) + "\n";
    frame += "receipt:" + receiptId + "\n\n";
    
    if (protocol.getConnection()) {
        protocol.getConnection()->sendFrameAscii(frame, '\0');
    }
}

void ProcessorInput::handleLogout(const std::vector<std::string>& args) {
    if (args.size() != 0) {
        std::cout << "Invalid logout command. Usage: logout " << std::endl;
        return;
    }
    int receiptInt = protocol.generateReceiptId();
    std::string receiptId = std::to_string(receiptInt);
    std::string frame = "DISCONNECT\n";
    frame += "receipt:" + receiptId + "\n\n";
    
    if (protocol.getConnection()) {
        protocol.getConnection()->sendFrameAscii(frame, '\0');
    }
    
    if (!protocol.getShouldTerminate()){
        protocol.addReceiptAction(std::stoi(receiptId), "DISCONNECT");
        protocol.waitForLogoutReceipt();
    }     
}

void ProcessorInput::handleReport(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Invalid report command. Usage: report {filename}" << std::endl;
        return;
    }
    if (!protocol.getConnected()) {
        std::cout << "Error: Not connected." << std::endl;
        return;
    }

    names_and_events data = parseEventsFile(args[0]);
    if (data.team_a_name.empty() && data.team_b_name.empty()) {
        return; 
    }

    std::string gameName = data.team_a_name + "_" + data.team_b_name;

    if (!protocol.isSubscribedTo(gameName)) {
        std::cout << "User is not subscribed to topic /" << gameName << std::endl;
        return;
    }

    std::string teamA = data.team_a_name;
    std::string teamB = data.team_b_name;
    
    for (const Event& event : data.events) {
        if (!protocol.getConnected()) {
            std::cout << "Connection lost. Stopping report." << std::endl;
            break;
        }


        std::string frame = "SEND\n";
        frame += "destination:/" + gameName + "\n"; 
        frame += "filename:" + args[0] + "\n";
        frame += "\n";

        frame += "user:" + protocol.getUsername()+ "\n";
        frame += "team a:" + teamA + "\n";
        frame += "team b:" + teamB + "\n";
        frame += "event name:" + event.get_name() + "\n";
        frame += "time:" + std::to_string(event.get_time()) + "\n";
        
        frame += "general game updates:\n";
        for (auto const& [key, val] : event.get_game_updates()) frame += key + ":" + val + "\n";
        frame += "team a updates:\n";
        for (auto const& [key, val] : event.get_team_a_updates()) frame += key + ":" + val + "\n";
        frame += "team b updates:\n";
        for (auto const& [key, val] : event.get_team_b_updates()) frame += key + ":" + val + "\n";

        frame += "description:\n";
        
        frame += event.get_discription() + "\n"; 
        
        ConnectionHandler* activeConn = protocol.getConnection();
        if (activeConn) {
            if (!activeConn->sendFrameAscii(frame, '\0')) {
                std::cout << "Server disconnected. Stopping report." << std::endl;
                protocol.setConnected(false);
                protocol.setShouldTerminate(true);
                break;
            }
        } else {
             std::cout << "Error: No active connection." << std::endl;
             break;
        }
    }
}

void ProcessorInput::handleSummary(const std::vector<std::string>& args) {
    if (args.size() != 3) {
        std::cout << "Usage: summary {game_name} {user} {file}" << std::endl;
        return;
    }
    std::string game_name = args[0];
    std::string user_name = args[1];
    std::string file_path = args[2];


    std::map<std::string,std::vector<Event>> eventsMap = protocol.getUserEvents(user_name);
    
    if (eventsMap.find(game_name) == eventsMap.end()) { 
         std::cout << "No updates from " << user_name << " for game " << game_name << std::endl;
         return;
    }

    const std::vector<Event>& events = eventsMap.at(game_name);
    if (events.empty()) return;

    std::map<std::string, std::string> general_stats;
    std::map<std::string, std::string> team_a_stats;
    std::map<std::string, std::string> team_b_stats;

    std::string team_a_name = events[0].get_team_a_name();
    std::string team_b_name = events[0].get_team_b_name();

    for (const Event& event : events) {
        for (const auto& pair : event.get_game_updates()) general_stats[pair.first] = pair.second;
        for (const auto& pair : event.get_team_a_updates()) team_a_stats[pair.first] = pair.second;
        for (const auto& pair : event.get_team_b_updates()) team_b_stats[pair.first] = pair.second;
    }

    std::ofstream file(file_path);
    if (!file.is_open()) {
        std::cout << "Error: Could not create file " << file_path << std::endl;
        return;
    }

    file << team_a_name << " vs " << team_b_name << "\n";
    file << "Game stats:\n";
    file << "General stats:\n";
    for (const auto& pair : general_stats) 
        if (!pair.second.empty()) 
            file << pair.first << ": " << pair.second << "\n";
    

    file << team_a_name << " stats:\n";
    for (const auto& pair : team_a_stats) 
        if (!pair.second.empty()) 
            file << pair.first << ": " << pair.second << "\n";

    file << team_b_name << " stats:\n";
    for (const auto& pair : team_b_stats) 
        if (!pair.second.empty()) 
            file << pair.first << ": " << pair.second << "\n";

    file << "Game event reports:\n";
    for (const Event& event : events) {
        file << event.get_time() << " - " << event.get_name() << ":\n";
        
        std::string desc = event.get_discription();
        std::string prefix = "description:\n";
        if (desc.find(prefix) == 0) 
            desc = desc.substr(prefix.length());
        
        file << desc << "\n";
    }
    file.close();
}

bool ProcessorInput::sendFrame(ConnectionHandler& connection, const std::string& frame) {
    return connection.sendFrameAscii(frame, '\0');
}