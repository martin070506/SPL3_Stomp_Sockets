package bgu.spl.net.srv;

import java.security.Key;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import javax.xml.crypto.Data;

import bgu.spl.net.impl.data.User;

public class ConnectionsImpl<T> implements Connections<String> {

    private ConcurrentHashMap<Integer, ConnectionHandler<String>> activeConnections;
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, Integer>> channelSubscribersConnectionId; 
    private ConcurrentHashMap<Integer, SubscriptionPair> subscriptionIdToPair; 
    private ConcurrentHashMap<Integer,User> loggedInUsers;
    private int connectionIdCounter;


    public ConnectionsImpl() {
        this.activeConnections = new ConcurrentHashMap<>();
        this.channelSubscribersConnectionId = new ConcurrentHashMap<>();
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
                
                int subscriptionId = subscribers.get(connectionId);

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
                subscriptionIdToPair.remove(subscriptionId);
            }
    }


    public boolean connect(int connectionId, String userName, String password) { 
       try {
        loggedInUsers.put(connectionId,new User(connectionId, userName, password));
        User user = loggedInUsers.get(connectionId);
        user.login();
       } catch (Exception e){}

       return true; 
    }

    @Override
    public void addConnection(int connectionId, ConnectionHandler<String> handler) { 

        activeConnections.put(connectionId, handler);
    }


    public void subscribe(String channel, int connectionId,int subscriptionId) {

        if (!channelSubscribersConnectionId.containsKey(channel)) {
            channelSubscribersConnectionId.putIfAbsent(channel, new ConcurrentHashMap<Integer, Integer>());
            System.out.println("Created Channel");
        }
        
        channelSubscribersConnectionId.get(channel).put(connectionId, subscriptionId);
        subscriptionIdToPair.putIfAbsent(subscriptionId, new SubscriptionPair(connectionId, channel));   
        printMap(channelSubscribersConnectionId.get(channel));
    }


    public void unsubscribe(int subscriptionId) {
       
        SubscriptionPair pair = subscriptionIdToPair.get(subscriptionId);
        if (pair == null)
            return;

        String channel = pair.getChannel();
        int connectionId = pair.getUserId();

        channelSubscribersConnectionId.get(channel).remove(connectionId);
        subscriptionIdToPair.remove(subscriptionId);
    }
    
    public boolean isUserConnectedById(int connectionId) {
        return loggedInUsers.containsKey(connectionId);
    }
    
    public boolean isSubscribed(String channel, int connectionId) {
       
        ConcurrentHashMap<Integer, Integer> subs = channelSubscribersConnectionId.get(channel);
       
        return (subs != null && subs.containsKey(connectionId));
    }

    public boolean isUserConnectedByUserName(String userName){
        for (User user : loggedInUsers.values()){
            if (user.name.equals(userName))
                return true;
        }
        return false;
    }
    public void printMap(ConcurrentHashMap<Integer,Integer> map) {

        map.forEach((k, v) -> System.out.println(k + ":" + v));
    }
}
