#pragma once
#include <string>
#include <vector>


// Forward declarations
class StompProtocol;
class ConnectionHandler;

class ProcessorInput {
public:
    // Static method or instance method to handle logic
    void process(const std::string& input);
    ProcessorInput(StompProtocol& protocol,ConnectionHandler& connection);
private:
    StompProtocol &protocol;
    ConnectionHandler& connection;
    // Helper functions to keep code clean
    void handleJoin(const std::vector<std::string>& args);
    void handleExit(const std::vector<std::string>& args);
    void handleReport(const std::vector<std::string>& args);
    void handleLogout(const std::vector<std::string>& args);
    bool isValidJoinCommand(const std::vector<std::string>& args);
    bool isValidExitCommand(const std::vector<std::string>& args);
    bool isValidLogoutCommand(const std::vector<std::string>& args);
};