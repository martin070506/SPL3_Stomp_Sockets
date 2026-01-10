package bgu.spl.net.srv;

public class SubscriptionPair {
    private int userId;
    private String channel;

    public SubscriptionPair(int userId, String channel) {
        this.userId = userId;
        this.channel = channel;
    }

    public int getUserId() { return userId; }
    public String getChannel() { return channel; }
}
