#pragma once
#include <string>
#include <vector>


class StompProtocol;
class ConnectionHandler;

class ProcessorInput {
public:
    void process(const std::string& input);
    ProcessorInput(StompProtocol& protocol,ConnectionHandler& connection);
private:
    StompProtocol &protocol;
    ConnectionHandler& connection;
    void handleJoin(const std::vector<std::string>& args);
    void handleExit(const std::vector<std::string>& args);
    void handleReport(const std::vector<std::string>& args);
    void handleLogout(const std::vector<std::string>& args);
    void handleSummary(const std::vector<std::string>& args);
    bool sendFrame(ConnectionHandler &connection, const std::string &frame);
};