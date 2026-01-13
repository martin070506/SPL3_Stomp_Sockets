#include <iostream>
class ProcessorInput{
    private:
        std::string ParseValueFromRequest(std::string &clientRequest, std::string &value);
        std::string ProcessConnectRequest(std::string& clientRequest);
        std::string ProcessSubscribeRequest(std::string& clientRequest);
        std::string ProcessUnsubscribeRequest(std::string& clientRequest);
        std::string ProcessSendRequest(std::string& clientRequest);
        std::string ProcessSummarizeRequest(std::string &clientRequest);
        bool isValidLoginCommand(const std::string& line);                                                


    public:
        ProcessorInput();
        std::string ProcessRequest(std::string &clientRequest);
};