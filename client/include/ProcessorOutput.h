#pragma once

#include <string>
#include <map>
#include "StompProtocol.h" // חייבים להכיר את הפרוטוקול כדי להחזיק רפרנס אליו

class ProcessorOutput {
private:
    StompProtocol& protocol;
    ConnectionHandler& connection; // רפרנס לפרוטוקול (המוח) שמנהל את ה-State ואת החיבור

    // --- פונקציות עזר פנימיות (Helpers) ---
    // הפונקציה הראשית שמפרקת את הפריים ומנתבת לפונקציות הטיפול
    void process(const std::string& frame);

    // מטפלים ספציפיים לכל סוג פקודה
    void handleConnected(const std::map<std::string, std::string>& headers);
    void handleMessage(const std::map<std::string, std::string>& headers, const std::string& body);
    void handleReceipt(const std::map<std::string, std::string>& headers);
    void handleError(const std::map<std::string, std::string>& headers, const std::string& body);

public:
    // בנאי: מקבל רק את הפרוטוקול.
    // (את החיבור הוא יקבל דרך protocol.getConnection() בתוך הפונקציה run)
    ProcessorOutput(StompProtocol& protocol,ConnectionHandler& connection);
    
    // הפונקציה שרצה בת'רד הנפרד (הלולאה האינסופית)
    void run();
};