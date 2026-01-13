#include "ConnectionHandler.h"
#include "StompProtocol.h" 

class ProcessorOutput {
private:
    ConnectionHandler& ch; 
    StompProtocol& protocol; 

public:
    ProcessorOutput(ConnectionHandler& ch, StompProtocol& protocol);
    void run();
};