package bgu.spl.net.srv;

import bgu.spl.net.api.StompMessagingProtocol;

public class StompProtocol<T> implements StompMessagingProtocol<String> {
    public boolean shouldTerminate=false;
    private Connections<String> connections;
    private int connectionId;
    private static int msgIdCounter = 0;

    /**
	 * Used to initiate the current client protocol with it's personal connection ID and the connections implementation
	**/
    
    public void start(int connectionId, Connections<String> connections){
         this.connectionId=connectionId;
         this.connections=connections;
         shouldTerminate=false;
    }
    
    public void process(String message){
        String command = message.split("\n")[0];
        switch(command) {
            case "CONNECT":
                System.out.println("CONNECT");
                handleConnectFrame(message);
                break;
            case "DISCONNECT":
                System.out.println("DISCONNECT");
                handleDisconnectFrame(message);
                break;
            case "SEND":
                System.out.println("SEND");
                handleSendFrame(message);
                break;
            case "SUBSCRIBE":
                System.out.println("SUBSCRIBE");
                handleSubscribeFrame(message);
                break;
            case "UNSUBSCRIBE":
                System.out.println("UNSUBSCRIBE");
                handleUnsubscribe(message);
                break;
            default:
                System.out.println("ERROR");
                sendErrorFrame("Some Error Message In Initial Connection"); //TODO refactor to send actual error messages
                break;
        }

    }
    public boolean shouldTerminate(){
        return shouldTerminate;
    }

    public void close(){
        shouldTerminate=true;
        connections.disconnect(connectionId);
    }

    /**
     * @param message
     * @param receiptId
     * @param login
     * @param passcode
     * @param version
     * @param host
        Handles the CONNECT frame according to the specs 
        currently when connecting we dont load any data like the users subscriptions
        again, i assume its done at the database level
        so like at initial starting of the server, the connection class is initialized with all the data needed 
        like subscriptions for each user that signed up
    */
    private void handleConnectFrame(String message) {
        // Implement CONNECT frame handling
        String login = getHeaderValue(message, "login");
        String receiptId = getHeaderValue(message, "receipt");
        String passcode = getHeaderValue(message, "passcode");
        String version = getHeaderValue(message, "accept-version");
        String host = getHeaderValue(message, "host");


         if (login == null || passcode == null || version == null || host == null) {
            // Construct Error Headers
            String errorHeaderBlock = "message:Missing required headers\n";
            if (receiptId != null) 
                errorHeaderBlock += "receipt-id:" + receiptId + "\n";

            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "Missing required headers in CONNECT frame" + 
                                "\u0000";

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            return;
        }

       // Check if user is already logged in
       // we dont need to check by id , because that specific problem is solved Client side by not allowing multiple connections with same client
        if (isUserConnectedByUserName(login)) {
            // 1. Prepare the Headers
            String errorHeaders = "message:User already logged in\n";
    
            // CORRECTION: Check if the client asked for a receipt
            if (receiptId != null) 
                errorHeaders += "receipt-id:" + receiptId + "\n";

            // 2. Construct the Frame
            String errorBody = "User already logged in";
            String errorFrame = "ERROR\n" +
                                errorHeaders +
                                "\n" + 
                                errorBody + 
                                "\u0000";   

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            
            return;
        }
        
        if (connections.connect(connectionId, login, passcode)) {
            
            String connectedFrame = "CONNECTED\n" +
                                    "version:1.2\n" +
                                    "\n" + 
                                    "\u0000";
                                    
            connections.send(connectionId, connectedFrame);

        } 
        else {
            String errorHeaderBlock = "message:Wrong password\n";
            if (receiptId != null) 
                errorHeaderBlock += "receipt-id:" + receiptId + "\n";

            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "Password does not match" + 
                                "\u0000";

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
        }
    }


    /**
     * 
     * @param message
     * @param receiptId
     * Handles the DISCONNECT frame according to the specs
     * currently when a client sends a DISCONNECT frame we just disconnect him
     * and we do not change any of his subscriptions or anything like that
     * so i believe we somehow store that in the database client side
     */
    private void handleDisconnectFrame(String message) {

        String receiptId = getHeaderValue(message, "receipt");

       if (receiptId == null) {
            String errorHeaderBlock = "message:Missing required headers\n";

            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "Missing required headers in DISCONNECT frame" + 
                                "\u0000";

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            return;
        }

        if (receiptId != null) {
            String receiptFrame = "RECEIPT\n" +
                                "receipt-id:" + receiptId + "\n" +
                                "\n" + 
                                "\u0000"; 
            connections.send(connectionId, receiptFrame);
        }

        connections.disconnect(connectionId);
        shouldTerminate = true; 
    }

    private void handleSendFrame(String message) {
    String destination = getHeaderValue(message, "destination");
    String receiptId = getHeaderValue(message, "receipt");
    String body = getBody(message);

    if (!isUserConnectedById(connectionId)) {
        String errorString = generateErrorString("User not logged in", 
                                                 "You must be logged in to send messages.", 
                                                 receiptId);
        sendErrorFrame(errorString);
        connections.disconnect(connectionId);
        shouldTerminate = true; 
        return;
    }

    if (destination == null || destination.isEmpty()) {
        String errorString = generateErrorString("Malformatted frame", 
                                                 "No destination header found", 
                                                 receiptId);
        sendErrorFrame(errorString);
        connections.disconnect(connectionId);
        shouldTerminate = true;
        return;
    }

    if (!connections.isSubscribed(destination, connectionId)) {
        String errorString = generateErrorString("Access Denied", 
                                                 "User is not subscribed to topic " + destination, 
                                                 receiptId);
        sendErrorFrame(errorString);
        connections.disconnect(connectionId);
        shouldTerminate = true;
        return;
    }

   
    // the 'connections' object handles the broadcasting logic ([display message id, and subscription id for each receiver])
    connections.send(destination, body);

    if (receiptId != null) {
        String receiptFrame = "RECEIPT\n" +
                              "receipt-id:" + receiptId + "\n" +
                              "\n" + 
                              "\u0000"; 
        connections.send(connectionId, receiptFrame);
    }
}

   
    

   
    /**
    @param message the full message received
    @param destination the destination header value
    @param receiptId the receipt header value
    @param subscriptionId the id header value
    The method gets those and handles them accordingly
    -REMINDER- the client only sends the channels name as destination for example "join usa/mexico"
    so we need on the client side somehow manage which channels hes subscribe to, and for each channel hes subscribe to he needs to hold his subscription id for that channel
    so again when the client says "leave usa/mexico" we know which subscription id to send
     */
    private void handleSubscribeFrame(String message){

        String destination= getHeaderValue(message, "destination");
        String receiptId= getHeaderValue(message, "receipt");
        String subscriptionId= getHeaderValue(message, "id");

        if (destination == null || subscriptionId == null){
            String errorHeaderBlock = "message:Missing required headers\n";
            if (receiptId != null) 
                errorHeaderBlock += "receipt-id:" + receiptId + "\n";

            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "Missing required headers in SUBSCRIBE frame" + 
                                "\u0000";

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            return;
        }

        int receiptIdNum, subIdNum;
        try {
            receiptIdNum=Integer.parseInt(receiptId);
            subIdNum=Integer.parseInt(subscriptionId);
        } catch (NumberFormatException e){
            String errorHeaderBlock = "message:Invalid header format\n";
            if (receiptId != null) 
                errorHeaderBlock += "receipt-id:" + receiptId + "\n";

            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "Invalid header format in SUBSCRIBE frame" + 
                                "\u0000";

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            return;
        }

        if (!isUserConnectedById(connectionId)) {
            String errorHeaderBlock = "message:User not connected\n";
            if (receiptId != null) 
                errorHeaderBlock += "receipt-id:" + receiptId + "\n";

            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "User not connected" + 
                                "\u0000";

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            return;
        }

        connections.subscribe(destination,connectionId,subIdNum);
        if (receiptId != null) {
            String receiptFrame = "RECEIPT\n" +
                                "receipt-id:" + receiptId + "\n" +
                                "\n" + 
                                "\u0000"; 
            connections.send(connectionId, receiptFrame);
        }
    }

    private void handleUnsubscribe(String message){
        String subscriptionId= getHeaderValue(message, "id");
        String receiptId= getHeaderValue(message, "receipt");

        if (!isUserConnectedById(connectionId)) {
            String errorHeaderBlock = "message:User not connected\n";
            if (receiptId != null) 
                errorHeaderBlock += "receipt-id:" + receiptId + "\n";
            
            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "User not connected" + 
                                "\u0000";
            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            return;
        }

        if (subscriptionId == null) {
            String errorHeaderBlock = "message:Missing required headers\n";
            if (receiptId != null) 
                errorHeaderBlock += "receipt-id:" + receiptId + "\n";
            
            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "Missing required headers in UNSUBSCRIBE frame" + 
                                "\u0000";

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            return;
        }

        int subIdNum;
        try {
            subIdNum = Integer.parseInt(subscriptionId);
        } catch (NumberFormatException e){
            String errorHeaderBlock = "message:Invalid header format\n";
            if (receiptId != null) 
                errorHeaderBlock += "receipt-id:" + receiptId + "\n";

            String errorFrame = "ERROR\n" +
                                errorHeaderBlock +
                                "\n" +
                                "Invalid header format in UNSUBSCRIBE frame" + 
                                "\u0000";

            sendErrorFrame(errorFrame);
            connections.disconnect(connectionId);
            return;
        }

        //I looked at the pdf and searched for what to do if client unsubscribes from somewhere he isnt subscribed to
        // but couldnt find anything so i assumed its just a no-op, anyways in the unsubscribe method it wont throw an error
        //but i dont think its a possible scenario because the client side handles giving the server side the id for the subscription, 
        // so he cannot give us something he isnt subscribed to

        connections.unsubscribe(subIdNum);
        if (receiptId != null) { 
            String receiptFrame = "RECEIPT\n" +
                                "receipt-id:" + receiptId + "\n" +
                                "\n" + 
                                "\u0000"; 
            connections.send(connectionId, receiptFrame);
        }
    }

    private void sendErrorFrame(String errorFrame){
           connections.send(this.connectionId, errorFrame);
    }
    
    private String getHeaderValue(String message, String headerName) {
        String[] lines = message.split("\n");
        for (String line : lines) {
            if (line.startsWith(headerName + ":")) 
                return line.substring(headerName.length() + 1);

            if (line.isEmpty()) 
                break; 
        }
        
        return null;
    }

    private String getBody(String message) {
    int splitIndex = message.indexOf("\n\n");
    if (splitIndex == -1) 
        return ""; 

        String body = message.substring(splitIndex + 2);
        return body.replace("\u0000", ""); 
    }

    private String generateUniqueId() {
        return String.valueOf(msgIdCounter++);
    }

    private boolean isUserConnectedByUserName(String userName) {
        return connections.isUserConnectedByUserName(userName);
    }
    private boolean isUserConnectedById(int connectionId) {
        return connections.isUserConnectedById(connectionId);
    }

    private String generateErrorString(String messageHeader, String body, String receiptId) {
        String headerBlock = "message:" + messageHeader + "\n";
        
        // Include receipt-id in the error if the user asked for one
        if (receiptId != null)
            headerBlock += "receipt-id:" + receiptId + "\n";

        return "ERROR\n" +
            headerBlock +
            "\n" +
            body + 
            "\u0000";
    }
    
}
