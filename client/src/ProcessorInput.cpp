#include "../include/ProcessorInput.h"
#include "../include/StompProtocol.h"
#include "../include/ConnectionHandler.h"
#include <iostream>
#include <sstream>
#include <fstream>

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
    else if (command == "exit") 
        handleExit(args);
    else if (command == "logout") 
        handleLogout(args);
    else if(command =="report"){
        handleReport(args);
    }
    else if(command =="summary"){
        handleSummary(args);
    }
    else if(protocol.getConnected()==false){
        std::cout << "User Must Login first" <<std::endl;
    }
    else{
        std::cout << "Invalid Command, Enter Something Valid" <<std::endl;
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
    frame += "receipt:" + receiptId + "\n";
    frame += "\n";

    protocol.addReceiptAction(receiptIdInt, "join " + gameName); 
    protocol.addSubscription(gameName, subId);

    bool success = true;
    if (!sendFrame(connection, frame)) 
        success = false;

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }
}   


void ProcessorInput::handleExit(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        std::cout << "Invalid exit command. Usage: exit {game_name}" << std::endl;
        return;
    }
    
    std::string gameName = args[0];

    if (!protocol.isSubscribedTo(gameName)) {
        std::cout << "User is not subscribed to channel " << gameName << std::endl;
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
    
    bool success = true;
    if (!sendFrame(connection, frame))
        success = false;

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }
}

void ProcessorInput::handleLogout(const std::vector<std::string>& args) {
    if (args.size() != 0) {
        std::cout << "Invalid logout command. Usage: logout " << std::endl;
        return;
    }
    int receiptInt=protocol.generateReceiptId();
    std::string receiptId = std::to_string(receiptInt);
    std::string frame = "DISCONNECT\n";
    frame += "receipt:" + receiptId + "\n\n";
    
    bool success = true;
    if (!sendFrame(connection, frame))
        success = false;

    if (!success) {
        std::cout << "Disconnected. Could not send frame." << std::endl;
        protocol.setShouldTerminate(true);
    }
    if (!protocol.getShouldTerminate()){
        protocol.addReceiptAction(std::stoi(receiptId), "DISCONNECT");
        protocol.waitForLogoutReceipt();

    }
        
}

void ProcessorInput::handleReport(const std::vector<std::string>& args){
    if (args.size() != 1) {
        std::cout << "Invalid logout command. Usage: report {filename} " << std::endl;
        return;
    }
    names_and_events data = parseEventsFile(args[0]);

   
    std::string teamA = data.team_a_name;
    std::string teamB = data.team_b_name;
    
   
    for (const Event& event : data.events) {
        
        
        std::string frame = "SEND\n";
        frame += "destination:/" + data.team_a_name + "_" + data.team_b_name + "\n"; // Example topic name
        frame += "\n"; // Empty line between headers and body

        // Body of the frame (The Event Data)
        frame += "user: " + protocol.getUsername()+ "\n";
        frame += "team a: " + teamA + "\n";
        frame += "team b: " + teamB + "\n";
        frame += "event name: " + event.get_name() + "\n";
        frame += "time: " + std::to_string(event.get_time()) + "\n";
        
        frame += "general game updates:\n";
        for (auto const& [key, val] : event.get_game_updates()) {
            frame += key + ":" + val + "\n";
        }

        frame += "team a updates:\n";
        for (auto const& [key, val] : event.get_team_a_updates()) {
            frame += key + ":" + val + "\n";
        }
        
        frame += "team b updates:\n";
        for (auto const& [key, val] : event.get_team_b_updates()) {
            frame += key + ":" + val + "\n";
        }

        frame += "description:\n" + event.get_discription() + "\n";
        
        // 5. Send the frame
        connection.sendFrameAscii(frame, '\0');
        
        // std::cout << "Sent event: " << event.get_name() << std::endl; // TODO: remove
    }
}

void ProcessorInput::handleSummary(const std::vector<std::string>& args){
    // 1. Validate Arguments
    if(args.size() != 3){
        std::cout << "Incorrect format, should be 'summary {game_name} {user} {file}'" << std::endl;
        return;
    }

    std::string game_name = args[0];
    std::string user_name = args[1];
    std::string file_path = args[2];

    // 2. Validate User Existence (Safe Check)
    // If we don't check this, accessing getUserEvents might throw or create garbage data
    if(!protocol.isSubscribedTo(game_name)){
        std::cout << "You are not subscribed to: " << game_name << std::endl;
        return;
    }
    if(protocol.getUserEvents(user_name).empty()){ 
         std::cout << "Has no updates from user: " << user_name << std::endl;
         return;
    }
    

    // 3. Validate Game Existence
    // Check if the specific game exists in the user's map
    if(protocol.getUserEvents(user_name).count(game_name) == 0){
        std::cout << "User " << user_name << " has no updates for game: " << game_name << std::endl;
        return;
    }

    // 4. Retrieve Events
    const std::vector<Event> events = protocol.getUserEvents(user_name).at(game_name);

    // --- SEGFAULT FIX IS HERE ---
    // Even if the game key exists, the vector might be empty.
    if(events.empty()){
        std::cout << "Error: Game '" << game_name << "' exists but has no stored events." << std::endl;
        return;
    }
    // ----------------------------

    // 5. Data Processing (Now safe to access events[0])
    std::map<std::string, std::string> general_stats;
    std::map<std::string, std::string> team_a_stats;
    std::map<std::string, std::string> team_b_stats;

    std::string team_a_name = events[0].get_team_a_name();
    std::string team_b_name = events[0].get_team_b_name();

    for (const Event& event : events){
        for(const auto& pair : event.get_game_updates()){
            general_stats[pair.first] = pair.second;
        }
        for(const auto& pair : event.get_team_a_updates()){
            team_a_stats[pair.first] = pair.second;
        }
        for(const auto& pair : event.get_team_b_updates()){
            team_b_stats[pair.first] = pair.second;
        }
    }

    // 6. Write to File
    std::ofstream file(file_path);
    if (!file.is_open()) {
        std::cout << "Error: Could not create file " << file_path << std::endl;
        return;
    }

    file << team_a_name << " vs " << team_b_name << "\n";
    file << "Game stats:\n";
    
    file << "General stats:\n";
    for (const auto& pair : general_stats) {
        file << pair.first << ": " << pair.second << "\n";
    }

    file << team_a_name << " stats:\n";
    for (const auto& pair : team_a_stats) {
        file << pair.first << ": " << pair.second << "\n";
    }

    file << team_b_name << " stats:\n";
    for (const auto& pair : team_b_stats) {
        file << pair.first << ": " << pair.second << "\n";
    }

    file << "Game event reports:\n";
    for (const Event& event : events) {
        file << event.get_time() << " - " << event.get_name() << ":\n\n";
        file << event.get_discription() << "\n\n";
    }

    file.close();
}


bool ProcessorInput::sendFrame(ConnectionHandler& connection, const std::string& frame) {
        return connection.sendBytes(frame.c_str(), frame.length()) &&
        connection.sendBytes("\0", 1);
}


