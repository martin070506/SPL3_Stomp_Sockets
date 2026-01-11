package bgu.spl.net.srv;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import bgu.spl.net.api.MessageEncoderDecoder;

public class StompEncoderDecoder implements MessageEncoderDecoder<String> {

    private byte[] bytesData=new byte[1<<10];
    int len = 0;
    public String decodeNextByte(byte nextByte){
        if(nextByte== '\u0000'){ 
            addByte(nextByte);
            return DecodeMessage(bytesData); // if actual message we return it
        }
        return null; //still no full message

    }

    /**
     * encodes the given message to bytes array
     *
     * @param message the message to encode
     * @return the encoded bytes
     */
    @Override
    public byte[] encode(String message) {
        return (message + '\u0000').getBytes(); //uses utf8 by default
    }
    
    private String DecodeMessage(byte[] bytes){
       String result = new String(bytes, 0, len, StandardCharsets.UTF_8);
        len = 0;
        return result;
    }

    private void addByte(byte Byte){
        if (len == bytesData.length)
            bytesData=Arrays.copyOf(bytesData,len*2);
            
        bytesData[len++]=Byte;
    }
    
}
