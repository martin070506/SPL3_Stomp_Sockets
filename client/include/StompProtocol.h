#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "../include/event.h"
#include"../include/ConnectionHandler.h"
#include "../include/ProcessorInput.h"
#include "../include/ProcessorOutput.h"





class StompProtocol {
private:
    std::map<std::string, int> channelToSubId;
    std::string username;
    std::map<std::string, std::map<std::string, std::vector<Event>>> userToEvents;
    std::map<int,std::string> receiptActions;
    ConnectionHandler* connection;
    int subscriptionIdCounter;
    int receiptIdCounter;
   
    bool shouldTerminate;
    bool isConnected;
    bool isError;
    std::mutex mtx;
    std::mutex logoutMutex;
    std::condition_variable logoutCV;

public:
    StompProtocol();
    ~StompProtocol();

    void waitForConnection();

    void processOutput(std::string serverFrame, ConnectionHandler& connection); //
    void processReceipt(int receiptId);
    void addEvent(const Event& event, const std::string& username);

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
    void waitForLogoutReceipt();
    void signalLogoutComplete();
    bool getIsError();
    void setIsError(bool val);
    bool getConnected();
    void setConnected(bool status); //
    std::string getUsername();

    StompProtocol(const StompProtocol&) = delete;
    StompProtocol& operator=(const StompProtocol&) = delete;

    std::string getGameNameByReceiptId(int receiptId);
    std::string getCommandTypeByReceiptId(int receiptId);
    void close();
};

