#pragma once

#include <string>
#include <map>
#include "StompProtocol.h"

class ProcessorOutput {
private:
    StompProtocol& protocol;
    ConnectionHandler& connection; 

    void process(const std::string& frame);

    std::string trim(std::string& str);

    void handleConnected(const std::map<std::string, std::string>& headers);
    void handleMessage(const std::map<std::string, std::string>& headers, const std::string& body);
    void handleReceipt(const std::map<std::string, std::string>& headers);
    void handleError(const std::map<std::string, std::string>& headers, const std::string& body);

public:
    ProcessorOutput(StompProtocol& protocol,ConnectionHandler& connection);
    void run();
};