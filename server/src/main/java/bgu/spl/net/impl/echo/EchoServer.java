package bgu.spl.net.impl.echo;

import java.util.function.Supplier;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;
import bgu.spl.net.srv.Server;
import bgu.spl.net.srv.StompEncoderDecoder;
import bgu.spl.net.srv.StompProtocol;

public class EchoServer {

    public static void main(String[] args) {

        // you can use any server... 
        Supplier<StompMessagingProtocol<String>> protocolFactory = () -> new StompProtocol<String>();
        Supplier<MessageEncoderDecoder<String>> encdecFactory = () -> new StompEncoderDecoder();
        Connections<String> connections = new ConnectionsImpl<>();
        Server.threadPerClient(
                7777, //port
                protocolFactory, //protocol factory
                encdecFactory,
                connections //message encoder decoder factory
        ).serve();

        // Server.reactor(
        //         Runtime.getRuntime().availableProcessors(),
        //         7777, //port
        //         () -> new EchoProtocol<>(), //protocol factory
        //         LineMessageEncoderDecoder::new //message encoder decoder factory
        // ).serve();
        
    }
}
