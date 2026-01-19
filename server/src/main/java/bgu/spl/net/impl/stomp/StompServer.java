package bgu.spl.net.impl.stomp;

import java.util.function.Supplier;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.echo.EchoProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.Server;
import bgu.spl.net.srv.StompEncoderDecoder;
import bgu.spl.net.srv.StompProtocol;

public class StompServer {

    public static void main(String[] args) {
        // you can use any server... 
        Supplier<StompMessagingProtocol<String>> protocolFactory = () -> new StompProtocol<String>();
        Supplier<MessageEncoderDecoder<String>> encdecFactory = () -> new StompEncoderDecoder();
        // Server.threadPerClient(
        //         7777, //port
        //         protocolFactory, //protocol factory
        //         encdecFactory //message encoder decoder factory
        // ).serve();

        Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                7777, //port
                protocolFactory, //protocol factory
                encdecFactory //message encoder decoder factory
        ).serve();
    }
}
