package bgu.spl.net.srv;

import bgu.spl.net.api.StompMessagingProtocol;

public class StompProtocol<T> implements StompMessagingProtocol<String> {
    public boolean shouldTerminate=false;
    private Connections<String> connections;
    private int connectionId;

    /**
	 * Used to initiate the current client protocol with it's personal connection ID and the connections implementation
	**/
    public void start(int connectionId, Connections<String> connections){
         this.connectionId=connectionId;
         this.connections=connections;
         shouldTerminate=false;
    }
    
    public void process(String message){


    }
    public boolean shouldTerminate(){
        return shouldTerminate;
    }

    //Each protocol will handle the string differently and check the connections accordingly
    // or send the messages to certain channels accordingly
    // IM GOING TO SLEEP , I Did not write a lot of code i refactored some and mostly worked out the flow / logic 
    // GOODNIGHT!!!
    private void handleConnectFrame(String message){

    }
    private void handleDisconnectFrame(String message){
        
    }
    private void handleSendFrame(String message){
        
    }
    private void handleSubscribeFrame(String message){
        
    }
    private void handleUnsubscribe(String message){
        
    }
    
}
