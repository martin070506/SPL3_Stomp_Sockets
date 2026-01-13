package bgu.spl.net.srv;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public interface Connections<T> {
    

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    boolean connect(int connectionId,String username,String password);

    boolean isSubscribed(String channel, int connectionId);

    void subscribe(String channel,int connectionId , String subscriptionId);

    void unsubscribe(int subscriptionId);

    boolean isUserConnectedByUserName(String username) ;

    boolean isUserConnectedById(int connectionId);

    void addConnection(int connectionId, ConnectionHandler<T> connectionHandler);


}
