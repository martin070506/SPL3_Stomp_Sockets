
#include "../include/ProcessorInput.h"
#include "../include/ProcessorOutput.h"
#include "../include/ConnectionHandler.h"


class StompProtocol
{
    private:
        ConnectionHandler &connectionHandler;
        std::map<std::string, int> subscriptions;
        ProcessorInput &processorInput;
        
        

    public:
        StompProtocol(ConnectionHandler &connectionHandler);
        void processInput(std::string input);
        void processResponse(std::string response);
};
