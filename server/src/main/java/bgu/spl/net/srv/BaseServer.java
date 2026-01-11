package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocol;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.function.Supplier;

public abstract class BaseServer<T> implements Server<T> {

    private final int port;
    private final Supplier<StompMessagingProtocol<T>> stompProtocolFactory;
    private final Supplier<MessageEncoderDecoder<T>> encdecFactory;
    private Connections<T> activeConnections;
    private ServerSocket sock;
    private int idCounter;

    public BaseServer(
            int port,
            Supplier<StompMessagingProtocol<T>> protocolFactory,
            Supplier<MessageEncoderDecoder<T>> encdecFactory) {
        this.port = port;
        this.stompProtocolFactory = protocolFactory;
        this.encdecFactory = encdecFactory;
		this.sock = null;
        this.activeConnections = new ConnectionsImpl();
        int idCounter = 0;
        
    }

    @Override
    public void serve() {

        try (ServerSocket serverSock = new ServerSocket(port)) {
			System.out.println("Server started");

            this.sock = serverSock; //just to be able to close

            while (!Thread.currentThread().isInterrupted()) {

                Socket clientSock = serverSock.accept();
                final int currentUserId = idCounter; //so theres no problem in the finally block
                idCounter++;
                StompMessagingProtocol<T> protocol = stompProtocolFactory.get();
                BlockingConnectionHandler<T> handler = new BlockingConnectionHandler<T>(
                    clientSock,
                    encdecFactory.get(),
                    protocol) {
                // OVERRIDE RUN TO CATCH DISCONNECTS

                @Override
                public void run() {
                    try {
                        super.run();
                    } finally {
                        activeConnections.disconnect(currentUserId);
                        System.out.println("User ID " + currentUserId + " disconnected.");
                    }
                }
            };

                activeConnections.addConnection(currentUserId, handler);
                protocol.start(currentUserId, activeConnections);// Supposed to add it to the acvtive connections itself, whichever class implements it

                execute(handler);
            }
        } catch (IOException ex) {}

        System.out.println("server closed!!!");
    }

    @Override
    public void close() throws IOException {
		if (sock != null)
			sock.close();
    }

    protected abstract void execute(BlockingConnectionHandler<T>  handler);

}
