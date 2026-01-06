package bgu.spl.net.srv;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

public class ConnectionsImpl<T> implements Connections<T> {

    private ConcurrentHashMap<Integer, ConnectionHandler<T>> activeConnections;
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, Boolean>> channelSubscribers;
    private int connectionIdCounter;


    public ConnectionsImpl() {
        this.activeConnections = new ConcurrentHashMap<>();
        this.channelSubscribers = new ConcurrentHashMap<>();
        this.connectionIdCounter = 0;
    }


    @Override
    public boolean send(int connectionId, T msg){

        ConnectionHandler clientConnection = activeConnections.get(connectionId);
        if (clientConnection == null) 
            return false;
        
        clientConnection.send(msg);
        return true;
    }



    @Override
    public void send(String channel, T msg){

        ConcurrentHashMap<Integer, Boolean> subscribers = channelSubscribers.get(channel);
        if (subscribers != null)
            for (Integer connectionId : subscribers.keySet())
                send(connectionId, msg);

    }


    @Override
    public void disconnect(int connectionId){

        activeConnections.remove(connectionId);
        for (String channel : channelSubscribers.keySet()) 
            if (channelSubscribers.get(channel).contains(connectionId))
                channelSubscribers.get(channel).remove(connectionId);

    }

    
    public void addConnection(int connectionId, ConnectionHandler<T> handler) {

        activeConnections.put(connectionId, handler);
    }


    public void subscribe(String channel, int connectionId) {

        if (!channelSubscribers.containsKey(channel))
            channelSubscribers.putIfAbsent(channel, new ConcurrentHashMap<Integer, Boolean>());

        ConcurrentHashMap<Integer, Boolean> subscribers = channelSubscribers.get(channel);
        subscribers.put(connectionId, false); // Boolean is a dummy value (placeholder) to satisfy the Map interface.
    }


    public void unsubscribe(String channel, int connectionId) {

        ConcurrentHashMap<Integer, Boolean> subscribers = channelSubscribers.get(channel);
        subscribers.remove(connectionId);

        if (subscribers.isEmpty())
            channelSubscribers.remove(channel);
    }
    
}
