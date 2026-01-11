package bgu.spl.net.srv;

import java.security.Key;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<String> {

    private ConcurrentHashMap<Integer, ConnectionHandler<String>> activeConnections;
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, Integer>> channelSubscribersConnectionId; // channel -> (connectionId -> subscriptionId)
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, Integer>> channelSubscribersSubscriptionId; // channel -> (subscriptionId -> connectionId)
    private ConcurrentHashMap<Integer, SubscriptionPair> subscriptionIdToPair; // subscriptionId -> (channel, connectionId)
    private ConcurrentHashMap<Integer,String> loggedInUsers;
    private int connectionIdCounter;


    public ConnectionsImpl() {
        this.activeConnections = new ConcurrentHashMap<>();
        this.channelSubscribersConnectionId = new ConcurrentHashMap<>();
        this.channelSubscribersSubscriptionId = new ConcurrentHashMap<>();
        this.subscriptionIdToPair = new ConcurrentHashMap<>();
        this.loggedInUsers = new ConcurrentHashMap<>();
        this.connectionIdCounter = 0;
    }


    @Override
    public boolean send(int connectionId, String msg){

        ConnectionHandler clientConnection = activeConnections.get(connectionId);
        if (clientConnection == null) 
            return false;
        
        clientConnection.send(msg);
        return true;
    }



    @Override
    public void send(String channel, String msg) {
        ConcurrentHashMap<Integer, Integer> subscribers = channelSubscribersConnectionId.get(channel);

        if (subscribers != null) {
            String messageId = Integer.toString(java.util.UUID.randomUUID().hashCode());
            for (Integer connectionId : subscribers.keySet()) {
                
                Integer subscriptionId = subscribers.get(connectionId);

                String frame = "MESSAGE\n" +
                            "subscription:" + subscriptionId + "\n" +
                            "message-id:" + messageId + "\n" +
                            "destination:" + channel + "\n" +
                            "\n" +
                            msg +
                            "\u0000";
                send(connectionId,  frame); 
            }
        }
    }


    @Override
    public void disconnect(int connectionId){

        activeConnections.remove(connectionId);
        loggedInUsers.remove(connectionId);
        for (String channel : channelSubscribersConnectionId.keySet()) 
            if (channelSubscribersConnectionId.get(channel).containsKey(connectionId)){
                int subscriptionId = channelSubscribersConnectionId.get(channel).remove(connectionId);
                channelSubscribersSubscriptionId.get(channel).remove(subscriptionId);
                subscriptionIdToPair.remove(subscriptionId);
            }
    }

    //adds an *actual* connection as if a user sent a CONNECT packet with UN and PASS , actual login is in python and the packet  is gonna be handled in proccess (assumes it worked)
    public boolean connect(int connectionId, String userName, String password) { 
       try {
        // some processing to check if the userName and password are valid
        loggedInUsers.put(connectionId, userName);
       } catch (Exception e){}

       return false; //just a placeholder until i know how to access the user data base
    }

    //adds a *GENERAL* connection, as in someone is connected to the socket
    @Override
    public void addConnection(int connectionId, ConnectionHandler<String> handler) { 

        activeConnections.put(connectionId, handler);
    }


    public void subscribe(String channel, int connectionId,int subscriptionId) {

        if (!channelSubscribersConnectionId.containsKey(channel)) {
            channelSubscribersConnectionId.putIfAbsent(channel, new ConcurrentHashMap<Integer, Integer>());
            channelSubscribersSubscriptionId.putIfAbsent(channel, new ConcurrentHashMap<Integer, Integer>());
        }
           
        ConcurrentHashMap<Integer, Integer> subscribersByCid = channelSubscribersConnectionId.get(channel);
        subscribersByCid.put(connectionId, subscriptionId); 
        ConcurrentHashMap<Integer, Integer> subscribersBySid = channelSubscribersSubscriptionId.get(channel);
        subscribersBySid.put(subscriptionId, connectionId);

        subscriptionIdToPair.putIfAbsent(subscriptionId, new SubscriptionPair(connectionId, channel));

        //SOMETHING WITH DB
    }


    public void unsubscribe(int subscriptionId) {
       
        SubscriptionPair pair = subscriptionIdToPair.get(subscriptionId);
        if (pair == null)
            return;

        String channel = pair.getChannel();
        int connectionId = pair.getUserId();

        channelSubscribersConnectionId.get(channel).remove(connectionId);
        channelSubscribersSubscriptionId.get(channel).remove(subscriptionId);

        subscriptionIdToPair.remove(subscriptionId);
    }


    public boolean isUserConnectedByUserName(String userName) {
        return loggedInUsers.containsValue(userName);
    }
    
    public boolean isUserConnectedById(int connectionId) {
        return loggedInUsers.containsKey(connectionId);
    }
    
    public boolean isSubscribed(String channel, int connectionId) {
        ConcurrentHashMap<Integer, Integer> subs = channelSubscribersConnectionId.get(channel);
        return (subs != null && subs.containsKey(connectionId));
    }
}
