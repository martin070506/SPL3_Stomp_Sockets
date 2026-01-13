#include "../include/ProcessorOutput.h"
#include "../include/Event.h"
#include <iostream>
#include <sstream>
#include <vector>

ProcessorOutput::ProcessorOutput(StompProtocol& protocol, ConnectionHandler& ch) 
    : protocol(protocol), ch(ch) {}

void ProcessorOutput::run() {
    while (!protocol.shouldTerminate()) {
        
        std::string frame;
        
        if (!ch.getFrameAscii(frame, '\0')) {
            std::cout << "Disconnected from server (Socket closed)" << std::endl;
            protocol.terminate();
            break;
        }

        std::stringstream ss(frame);
        std::string command;
        
        // קריאת הפקודה (השורה הראשונה)
        std::getline(ss, command); 

        // קריאת ההדרים (Headers)
        std::string line;
        std::map<std::string, std::string> headers;
        
        // רצים עד שמגיעים לשורה ריקה
        while (std::getline(ss, line) && line != "") {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                headers[key] = value;
            }
        }

        // קריאת הגוף (Body)
        std::string body;
        char c;
        while(ss.get(c)) {
            body += c;
        }

        // --- טיפול בפקודות ---

        if (command == "CONNECTED") {
            std::cout << "Login successful" << std::endl;
            protocol.setConnected(true);
        }
        
        else if (command == "ERROR") {
            std::cout << "Error from server: " << std::endl;
            if (headers.count("message")) {
                std::cout << headers["message"] << std::endl;
            }
            std::cout << body << std::endl;
            
            protocol.terminate();
            ch.close();
        }
        
        else if (command == "RECEIPT") {
            if (headers.count("receipt-id")) {
                int receiptId = std::stoi(headers["receipt-id"]);
                protocol.processReceipt(receiptId); 
            }
        }
        
        else if (command == "MESSAGE") {
            std::string user = "";
            if (headers.count("user")) {
                user = headers["user"];
            }
            
            Event event(body); 
            protocol.addEvent(event, user);
        }
    }
}