package bgu.spl.net.srv;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.data.Database;
import bgu.spl.net.impl.data.LoginStatus;

public class StompProtocol<T> implements StompMessagingProtocol<String> {
    public boolean shouldTerminate=false;
    private Connections<String> connections;
    private int connectionId;
    private boolean isConnected = false;

    /**
	 * Used to initiate the current client protocol with it's personal connection ID and the connections implementation
	**/
    
    public void start(int connectionId, Connections<String> connections){
         this.connectionId = connectionId;
         this.connections = connections;
         shouldTerminate = false;
         isConnected = false;
    }
    
    public void process(String message){

        System.out.println("\nCommand Gotten:\n" + message);
        String command = message.split("\n")[0];
        switch(command) {
            case "CONNECT":
                handleConnectFrame(message);
                break;
            case "DISCONNECT":
                handleDisconnectFrame(message);
                break;
            case "SEND":
                handleSendFrame(message);
                break;
            case "SUBSCRIBE":
                handleSubscribeFrame(message);
                break;
            case "UNSUBSCRIBE":
                handleUnsubscribe(message);
                break;
            default:
                sendErrorFrame("Unknown Command", null); 
                break;
        }
    }
    
    public boolean shouldTerminate(){
        return shouldTerminate;
    }

    public void close() {
        shouldTerminate = true;
        isConnected = false;

        Database.getInstance().logout(connectionId);
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
        String login = getHeaderValue(message, "login");
        String passcode = getHeaderValue(message, "passcode");
        String receiptId = getHeaderValue(message, "receipt");

        if (login == null || passcode == null) {
            sendErrorFrame("Missing required headers", receiptId);
            close();
            return;
        }

        LoginStatus status = Database.getInstance().login(connectionId, login, passcode);
        
        switch (status) {
            case LOGGED_IN_SUCCESSFULLY:
            case ADDED_NEW_USER:
                isConnected = true; 
                System.out.println("Login Successful: " + login);
                
                String connectedFrame = "CONNECTED\n" +
                                        "version:1.2\n" +
                                        "\n" + 
                                        "\u0000";
                connections.send(connectionId, connectedFrame);
                
                if (receiptId != null) 
                    sendReceipt(receiptId);
                break;

            case WRONG_PASSWORD:
                sendErrorFrame("Wrong password", receiptId);
                close();
                break;

            case ALREADY_LOGGED_IN:
                sendErrorFrame("User already logged in", receiptId);
                close();
                break;

            case CLIENT_ALREADY_CONNECTED:
                sendErrorFrame("User already logged in", receiptId);
                close();
                break;
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

        if (receiptId != null) 
            sendReceipt(receiptId);

        close(); 
    }

    private void handleSendFrame(String message) {
        String receiptId = getHeaderValue(message, "receipt");
        String filename = getHeaderValue(message, "filename");
        String username = getHeaderValue(message, "user");
        if (!isConnected) {
            sendErrorFrame("User not logged in", receiptId);
            close();
            return;
        }

        String destination = getHeaderValue(message, "destination");
        String body = getBody(message);

        if (destination == null || destination.isEmpty()) {
            sendErrorFrame("No destination header found", receiptId);
            return;
        }

        if (!connections.isSubscribed(destination, connectionId)) {
            sendErrorFrame("User is not subscribed to topic " + destination, receiptId);
            return;
        }

        connections.send(destination, body);
        Database.getInstance().trackFileUpload(username , filename, destination);

        if (receiptId != null) 
            sendReceipt(receiptId);
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
    private void handleSubscribeFrame(String message) {
        String receiptId = getHeaderValue(message, "receipt");

        if (!isConnected) {
            sendErrorFrame("User not logged in", receiptId);
            close();
            return;
        }

        String destination = getHeaderValue(message, "destination");
        String subscriptionId = getHeaderValue(message, "id");

        if (destination == null || subscriptionId == null) {
            sendErrorFrame("Missing required headers", receiptId);
            return;
        }

        try {
            int subId = Integer.parseInt(subscriptionId);
            connections.subscribe(destination, connectionId, subId);
            
            if (receiptId != null) 
                sendReceipt(receiptId);
            
        } catch (NumberFormatException e) {
            sendErrorFrame("Invalid subscription ID", receiptId);
        }
    }

    private void handleUnsubscribe(String message) {
        String receiptId = getHeaderValue(message, "receipt");

        if (!isConnected) {
            sendErrorFrame("User not logged in", receiptId);
            close();
            return;
        }

        String subscriptionId = getHeaderValue(message, "id");

        if (subscriptionId == null) {
            sendErrorFrame("Missing required headers", receiptId);
            return;
        }

        try {
            int subId = Integer.parseInt(subscriptionId);
            connections.unsubscribe(subId);
            
            if (receiptId != null) 
                sendReceipt(receiptId);
            
        } catch (NumberFormatException e) {
            sendErrorFrame("Invalid subscription ID", receiptId);
        }
    }

    private void sendReceipt(String receiptId) {
        String frame = "RECEIPT\n" +
                       "receipt-id:" + receiptId + "\n" +
                       "\n" + 
                       "\u0000"; 
        connections.send(connectionId, frame);
    }
    
    private void sendErrorFrame(String messageBody, String receiptId) {
        String headers = "message:Error\n";
        if (receiptId != null) 
            headers += "receipt-id:" + receiptId + "\n";
        
        String frame = "ERROR\n" +
                       headers +
                       "\n" +
                       messageBody +
                       "\u0000";
        connections.send(connectionId, frame);
    }
    
    
    
    private String getHeaderValue(String message, String headerName) {
        String[] lines = message.split("\n");
        for (String line : lines) {
            if (line.startsWith(headerName + ":")) 
                return line.substring(headerName.length() + 1);
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
}
