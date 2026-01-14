#include "../include/StompProtocol.h"
#include "../include/ProcessorInput.h"
#include "../include/ProcessorOutput.h"
#include "../include/ConnectionHandler.h"
#include "../include/event.h"
#include <map>

StompProtocol::StompProtocol() :
    subscriptionIdCounter(0), 
    receiptIdCounter(0), 
    shouldTerminate(false),
    isConnected(false),
    isError(false),
    receiptActions(std::map<int,std::string>()),
    userToEvents(std::map<std::string, std::map<std::string, std::vector<Event>>>()),
    connection(nullptr),
    channelToSubId(std::map<std::string,int>()) {}

StompProtocol::~StompProtocol() {
    if (connection)
        connection->close();
    delete connection;
}

ConnectionHandler* StompProtocol::getConnection() {
    return connection;
}

// The Listener Thread calls this
void StompProtocol::waitForConnection() {
    while (!isConnected) {
        const short bufsize = 1024;
        char buf[bufsize];
        
        std::cout << "Please login (login host:port username password):" << std::endl;
        std::cin.getline(buf, bufsize);
        std::string line(buf);
        
        if (std::cin.eof()) {
             // Handle Ctrl+D immediately so we don't loop forever
             shouldTerminate = true;
             return;
        }

        std::stringstream ss(line);
        std::string command;
        ss >> command;

        if (command != "login") {
            std::cout << "Error: You must log in first." << std::endl;
            continue;
        }

        std::string hostPort, username, password;
        ss >> hostPort >> username >> password;

        size_t colonPos = hostPort.find(':');
        if (colonPos == std::string::npos) {
            std::cout << "Error: Invalid host:port format" << std::endl;
            continue;
        }

        std::string host = hostPort.substr(0, colonPos);
        short port = (short)std::stoi(hostPort.substr(colonPos + 1));

        // 1. Initialize the internal connection member
        if (connection) delete connection; // Safety clean up if retrying
        connection = new ConnectionHandler(host, port);

        if (!connection->connect()) {
            std::cerr << "Error: Could not connect to server " << host << ":" << port << std::endl;
            // Clean up immediately on failure so we can try again
            delete connection;
            connection = nullptr;
            continue;
        }

        // 2. Send CONNECT Frame (Manual Bootstrapping)
        std::string frame = "CONNECT\n";
        frame += "accept-version:1.2\n";
        frame += "host:stomp.cs.bgu.ac.il\n";
        frame += "login:" + username + "\n";
        frame += "passcode:" + password + "\n";
        frame += "\n"; 

        bool sent = connection->sendBytes(frame.c_str(), frame.length());
        sent = sent && connection->sendBytes("\0", 1); 

        if (!sent) {
            std::cout << "Error sending CONNECT frame." << std::endl;
            delete connection;
            connection = nullptr;
            continue;
        }

        // 3. Wait for Receipt
        std::string answer;
        if (!connection->getFrameAscii(answer, '\0')) {
             std::cout << "Error: Server disconnected during login." << std::endl;
             delete connection;
             connection = nullptr;
             continue;
        }

        if (answer.find("CONNECTED") != std::string::npos) {
            std::cout << "Login successful." << std::endl;
            this->isConnected = true;
            // We return void. The 'connection' member is now ready.
            return; 
        } else {
            std::cout << "Login failed. Response:\n" << answer << std::endl;
            connection->close();
            delete connection;
            connection = nullptr;
        }
    }
}

void StompProtocol::processReceipt(int receiptId) {
    std::lock_guard<std::mutex> lock(mtx); 
    if (receiptActions.count(receiptId)) {
        std::string action = receiptActions[receiptId];
        std::cout << "Server confirmed: " << action << std::endl;

        if (action == "DISCONNECT") {
            isError = false;
            shouldTerminate = true;
            isConnected = false;
            if (connection) 
                connection->close(); 
        }  
        receiptActions.erase(receiptId);
    }
}

void StompProtocol::addSubscription(const std::string& channel, int id) {
    std::lock_guard<std::mutex> lock(mtx);
    channelToSubId[channel] = id;
}

void StompProtocol::removeSubscription(const std::string& channel) {
    std::lock_guard<std::mutex> lock(mtx);
    if (channelToSubId.count(channel)) 
        channelToSubId.erase(channel);
}

int StompProtocol::getSubscriptionId(const std::string& channel) {
    std::lock_guard<std::mutex> lock(mtx);
    if (channelToSubId.count(channel)) 
        return channelToSubId[channel];

    return -1; // לא קיים מנוי למשחק הזה
}

bool StompProtocol::isSubscribedTo(const std::string& channel) {
    std::lock_guard<std::mutex> lock(mtx);

    return channelToSubId.count(channel) > 0;
}

void StompProtocol::addEvent(const Event& event, const std::string& username) {
    std::lock_guard<std::mutex> lock(mtx);

    std::string gameName = event.get_team_a_name() + "_" + event.get_team_b_name();
    userToEvents[username][gameName].push_back(event);
}

void StompProtocol::addReceiptAction(int receiptId, const std::string& action) {
    std::lock_guard<std::mutex> lock(mtx);
    receiptActions[receiptId] = action;
}

int StompProtocol::generateSubId() {
    std::lock_guard<std::mutex> lock(mtx);
    return ++subscriptionIdCounter;
}

int StompProtocol::generateReceiptId() {
    std::lock_guard<std::mutex> lock(mtx);
    return ++receiptIdCounter;
}

ConnectionHandler* StompProtocol::getConnection() {
    return connection;
}

bool StompProtocol::getShouldTerminate() const {
    return shouldTerminate;
}

void StompProtocol::setShouldTerminate(bool val) {
    std::lock_guard<std::mutex> lock(mtx);
    shouldTerminate = val;
}

void StompProtocol::setConnected(bool status) {
    std::lock_guard<std::mutex> lock(mtx);
    isConnected = status;
}

bool StompProtocol::isSubscribedTo(const std::string& channel){
    return channelToSubId.count(channel)>0;
}

bool StompProtocol::getIsError() {
    return isError;
}

bool StompProtocol::setIsError(bool val) {
    isError = val;
}
