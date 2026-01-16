#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "../include/event.h"
#include"../include/ConnectionHandler.h"
#include "../include/ProcessorInput.h"
#include "../include/ProcessorOutput.h"


// Forward declarations to avoid circular includes


class StompProtocol {
private:
    // --- STATE VARIABLES ---
    // Map: Channel Name -> Subscription ID (e.g., "germany_spain" -> 78)
    std::map<std::string, int> channelToSubId;
    std::string username;
    // Map: Username -> List of events (Game updates received)
    std::map<std::string, std::map<std::string, std::vector<Event>>> userToEvents;
    std::map<int,std::string> receiptActions;
    ConnectionHandler* connection;
    // Helper counters
    int subscriptionIdCounter;
    int receiptIdCounter;
   
    // State flags
    bool shouldTerminate;
    bool isConnected;
    bool isError;
    std::mutex mtx;

public:
    StompProtocol();
    ~StompProtocol();

    // --- MAIN ENTRY POINTS ---
    // Called by Main Thread (Keyboard)
    void waitForConnection();

    // Called by Listener Thread (Socket)
    void processOutput(std::string serverFrame, ConnectionHandler& connection); //
    void processReceipt(int receiptId);
    void addEvent(const Event& event, const std::string& username);

    // --- GETTERS / SETTERS FOR PROCESSORS TO USE ---
    // The processors need access to modify these maps
    void addSubscription(const std::string& channel, int id); //
    void removeSubscription(const std::string& channel); //
    int getSubscriptionId(const std::string& channel); //
    bool isSubscribedTo(const std::string& channel);
    std::map<std::string,std::vector<Event>> getUserEvents(const std::string& username);
    void addReceiptAction(int receiptId, const std::string& action); //
    int generateSubId(); //
    int generateReceiptId(); //
    ConnectionHandler* getConnection(); 

    bool getShouldTerminate() const; //
    void setShouldTerminate(bool val); //

    bool getIsError();
    bool setIsError(bool val);
    
    void setConnected(bool status); //
    std::string getUsername();
};

