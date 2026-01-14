#include <iostream>
#include <thread>
#include "../include/StompProtocol.h"
#include "../include/ProcessorInput.h"
#include "../include/ProcessorOutput.h"

int main(int argc, char *argv[]) {
    
    // 1. Initialize Protocol
    StompProtocol protocol;

    while (!protocol.getIsError()) {
        protocol.setIsError(true);
        
        // 2. Phase 1: Login
        // Protocol handles the loop and stores the connection internally.
        protocol.waitForConnection();

        // Check if we actually connected or if the user quit (Ctrl+D)
        if (protocol.getConnection() == nullptr) 
            return 0;

        // ==========================================================
        // PHASE 2: GAME LOOP
        // ==========================================================

        // 1. Create Processors
        // We get the connection from the protocol to pass it to processors
        // (Assuming Processors accept ConnectionHandler& in constructor)
        ProcessorInput input(protocol, *protocol.getConnection());
        ProcessorOutput output(protocol, *protocol.getConnection());

        // 2. Start Listener Thread
        std::thread listenerThread(&ProcessorOutput::run, &output);

        // 3. Start Keyboard Loop
        while (!protocol.getShouldTerminate()) {
            const short bufsize = 1024;
            char buf[bufsize];
            
            std::cin.getline(buf, bufsize);
            std::string line(buf);

            if (line.empty()) continue;

            input.process(line);
        }

        // ==========================================================
        // PHASE 3: CLEANUP
        // ==========================================================
        // StompProtocol destructor will handle deleting the connection.
        // We just need to make sure the thread joins.
        
        // Force close to wake up the listener thread if it's stuck in read()
        if (protocol.getConnection())
            protocol.getConnection()->close();

        if (listenerThread.joinable()) 
            listenerThread.join();
    }

    return 0;
}
