#include <iostream>
#include <thread>
#include "../include/StompProtocol.h"
#include "../include/ProcessorInput.h"
#include "../include/ProcessorOutput.h"

int main(int argc, char *argv[]) {
    
    StompProtocol protocol;

    while (!protocol.getIsError()) {
        protocol.setIsError(true);
        protocol.setShouldTerminate(false);
        
        protocol.waitForConnection();
        if (protocol.getConnection() == nullptr) 
            return 0;

        ProcessorInput input(protocol, *protocol.getConnection());
        ProcessorOutput output(protocol, *protocol.getConnection());

        std::thread listenerThread(&ProcessorOutput::run, &output);

        while (!protocol.getShouldTerminate()) {
            const short bufsize = 1024;
            char buf[bufsize];
            
            std::cin.getline(buf, bufsize);
            std::string line(buf);

            if (line.empty()) 
                continue;

            input.process(line);
        }

        // Force close to wake up the listener thread if it's stuck in read
        if (protocol.getConnection())
            protocol.getConnection()->close();

        if (listenerThread.joinable()) 
            listenerThread.join();
    }
    delete protocol.getConnection();
    return 0;
}
