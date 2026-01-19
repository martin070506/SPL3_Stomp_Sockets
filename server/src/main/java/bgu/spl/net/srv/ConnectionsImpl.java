package bgu.spl.net.srv;

import java.security.Key;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import javax.xml.crypto.Data;

import bgu.spl.net.impl.data.User;

public class ConnectionsImpl<T> implements Connections<String> {

    private ConcurrentHashMap<Integer, ConnectionHandler<String>> activeConnections;
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, Integer>> channelSubscribers; // channel -> (connectionId -> subscriptionId)
    private ConcurrentHashMap<Integer, SubscriptionPair> subscriptionIdToPair; // subscriptionId -> (channel, connectionId)


    public ConnectionsImpl() {
        this.activeConnections = new ConcurrentHashMap<>();
        this.channelSubscribers = new ConcurrentHashMap<>();
        this.subscriptionIdToPair = new ConcurrentHashMap<>();
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
        ConcurrentHashMap<Integer, Integer> subscribers = channelSubscribers.get(channel);

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
    public void disconnect(int connectionId) {

        activeConnections.remove(connectionId);
        // loggedInUsers.remove(connectionId);
        for (String channel : channelSubscribers.keySet()) {
            ConcurrentHashMap<Integer, Integer> subs = channelSubscribers.get(channel);
            if (subs.containsKey(connectionId))
                subscriptionIdToPair.remove(subs.remove(connectionId));
        }
    }
    
    //adds a *GENERAL* connection, as in someone is connected to the socket
    @Override
    public void addConnection(int connectionId, ConnectionHandler<String> handler) { 

        activeConnections.put(connectionId, handler);
    }


    public void subscribe(String channel, int connectionId,int subscriptionId) {

        if (!channelSubscribers.containsKey(channel)) {
            channelSubscribers.putIfAbsent(channel, new ConcurrentHashMap<Integer, Integer>());
            System.out.println("Created Channel");
        }
        
        channelSubscribers.get(channel).put(connectionId, subscriptionId);
        subscriptionIdToPair.putIfAbsent(subscriptionId, new SubscriptionPair(connectionId, channel));   
        printMap(channelSubscribers.get(channel));
    }


    public void unsubscribe(int subscriptionId) {
       
        SubscriptionPair pair = subscriptionIdToPair.get(subscriptionId);
        if (pair == null)
            return;

        String channel = pair.getChannel();
        int connectionId = pair.getUserId();

        channelSubscribers.get(channel).remove(connectionId);
        subscriptionIdToPair.remove(subscriptionId);
    }
    
    
    public boolean isSubscribed(String channel, int connectionId) {
       
        ConcurrentHashMap<Integer, Integer> subs = channelSubscribers.get(channel);
       
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
        // Uses the built-in forEach method
        map.forEach((k, v) -> System.out.println(k + ":" + v));
    }
}
