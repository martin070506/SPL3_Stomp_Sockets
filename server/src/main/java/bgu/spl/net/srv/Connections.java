package bgu.spl.net.srv;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public interface Connections<T> {
    

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);
}
