package bgu.spl.net.impl.stomp;

import java.util.function.Supplier;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.impl.data.Database;
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

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("\nServer is shutting down... generating report.");
            Database.getInstance().printReport(); 
        }));
        
        if(args[1].equals("tpc")){
            Server.threadPerClient(
                Integer.parseInt(args[0]), //port
                protocolFactory, //protocol factory
                encdecFactory //message encoder decoder factory
            ).serve();
        }
        
        if(args[1].equals("reactor")){
            Server.reactor(
                Runtime.getRuntime().availableProcessors(),
                Integer.parseInt(args[0]), //port
                protocolFactory, //protocol factory
                encdecFactory //message encoder decoder factory
            ).serve();
        }
        Database.getInstance().printReport(); //initialize the database
    }
}
