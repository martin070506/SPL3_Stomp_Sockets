// #include "../include/ProccessorOutput.h"
// #include "../include/StompProtocol.h"
// #include "../include/event.h"
// #include <iostream>
// #include <sstream>
// #include <vector>

// ProcessorOutput::ProcessorOutput(ConnectionHandler& ch, StompProtocol& protocol) 
//     : ch(ch), protocol(protocol) {}

// void ProcessorOutput::run() {
//     while (!protocol.shouldTerminate()) {
//         std::string frame;

//         if (!ch.getFrameAscii(frame, '\0')) {
//             std::cout << "Disconnected from server (Socket closed)" << std::endl;
//             protocol.terminate();
//             break;
//         }

//         std::stringstream ss(frame);
//         std::string command;
//         std::getline(ss, command); 

//         std::string line;
//         std::map<std::string, std::string> headers;
//         while (std::getline(ss, line) && line != "" && line != "\r") {
//             size_t colonPos = line.find(':');
//             if (colonPos != std::string::npos) {
//                 std::string key = line.substr(0, colonPos);
//                 std::string value = line.substr(colonPos + 1);
//                 // ניקוי תווי \r אם יש
//                 if (!value.empty() && value.back() == '\r') value.pop_back();
//                 headers[key] = value;
//             }
//         }

//         // קריאת הגוף (Body)
//         std::string body;
//         // שארית הסטרים היא הגוף
//         char c;
//         while(ss.get(c)) {
//             body += c;
//         }
        
//         // --- Logic Handling ---

//         if (command == "CONNECTED") {
//             std::cout << "Login successful" << std::endl; // [cite: 322]
//             protocol.setConnected(true);
//         }
//         else if (command == "ERROR") {
//             std::cout << "Error from server: " << std::endl;
//             if (headers.count("message")) {
//                 std::cout << headers["message"] << std::endl; // [cite: 125]
//             }
//             std::cout << body << std::endl;
//             protocol.terminate();
//             ch.close();
//         }
//         else if (command == "RECEIPT") {
//             if (headers.count("receipt-id")) {
//                 int receiptId = std::stoi(headers["receipt-id"]);
//                 protocol.processReceipt(receiptId); // [cite: 104]
//             }
//         }
//         else if (command == "MESSAGE") {
//             // הודעה ממשתמש אחר על עדכון במשחק
//             // הפרוטוקול דורש לשמור את האירוע
//             std::string user = headers["user"]; // נניח שיש הדר כזה לפי הדוגמה ב-[cite: 384]
//             Event event(body); // שימוש בקונסטרקטור של Event שמפרסר את הגוף
            
//             // הדרישה: לעדכן את המשחק ואם רוצים לעשות summary אח"כ
//             protocol.addEvent(event, user);
            
//             // הדפסה חיה למסך אינה חובה לפי הדרישות היבשות ל-MESSAGE, 
//             // אבל הגיוני להציג משהו. עם זאת, הדרישה העיקרית היא join/exit ו-summary.
//             std::cout << "Update received for game: " << event.get_team_a_name() << " vs " << event.get_team_b_name() << std::endl;
//         }
//     }
// }