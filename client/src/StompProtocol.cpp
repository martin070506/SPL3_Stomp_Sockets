#include <iostream>
#include "../include/StompProtocol.h"
#include "../include/ProcessorInput.h"

StompProtocol::StompProtocol(ConnectionHandler &connectionHandler):
        connectionHandler(connectionHandler), 
        shouldTerminate_flag(false), 
        isConnected_flag(false),
        subscriptionIdCounter(0),
        receiptIdCounter(0),
        processorOutput(ProcessorOutput(connectionHandler, *this)),
        processorInput(ProcessorInput()) {}





void StompProtocol::processInput(std::string input) {
    std::string frame = processorInput.ProcessRequest(input);

    //the processInput function just creates the appropriate frame and sends it
    //for example  a subscribe request will be adressed immediately, because we dont need server confirmation to subscribe
    // so logic is gonna be done here, and frame creation is done in processorInput.
    // the connection.login request is done in the main function.

    //TODO: HANDLE LOGIC FOR DIFFERENT COMMANDS IF NEEDED
    
    if (!frame.empty()) {
        connectionHandler.sendFrameAscii(frame, '\0');
    }
}
void StompProtocol::addReceiptExpectation(int receiptId, std::string actionDescription) {
    std::lock_guard<std::mutex> lock(mtx); 
    receiptActions[receiptId] = actionDescription;
}

void StompProtocol::processReceipt(int receiptId) {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (receiptActions.count(receiptId)) {
        std::string action = receiptActions[receiptId];
        
        if (action == "DISCONNECT") {
            isConnected_flag = false;
            shouldTerminate_flag = true;
            connectionHandler.close(); [cite_start][cite: 175]
            std::cout << "Disconnected securely based on receipt." << std::endl;
        } 
        else {
            [cite_start] [cite: 343-344]
            std::cout << action << std::endl; 
        }
        
        receiptActions.erase(receiptId);
    }
}


void StompProtocol::addEvent(const Event& event, std::string username) {
    std::lock_guard<std::mutex> lock(mtx);
    
    std::string gameName = event.get_team_a_name() + "_" + event.get_team_b_name();
    
    userEvents[username][gameName].push_back(event);
}


int StompProtocol::getNewSubscriptionId() {
    std::lock_guard<std::mutex> lock(mtx);
    return ++subscriptionIdCounter;
}

int StompProtocol::getNewReceiptId() {
    std::lock_guard<std::mutex> lock(mtx);
    return ++receiptIdCounter;
}

void StompProtocol::addSubscription(std::string topic, int id) {
    std::lock_guard<std::mutex> lock(mtx);
    subscriptions[topic] = id;
}

void StompProtocol::removeSubscription(std::string topic) {
    std::lock_guard<std::mutex> lock(mtx);
    subscriptions.erase(topic);
}

int StompProtocol::getSubscriptionId(std::string topic) {
    std::lock_guard<std::mutex> lock(mtx);
    if (subscriptions.count(topic)) {
        return subscriptions[topic];
    }
    return -1; 
}



bool StompProtocol::shouldTerminate() {
    return shouldTerminate_flag;
}

bool StompProtocol::isConnected() {
    return isConnected_flag;
}

void StompProtocol::setConnected(bool status) {
    isConnected_flag = status;
}

void StompProtocol::terminate() {
    shouldTerminate_flag = true;
}