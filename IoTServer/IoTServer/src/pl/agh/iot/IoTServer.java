package pl.agh.iot;

import java.io.*;
import java.net.InetSocketAddress;

import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

import com.google.gson.Gson;

public class IoTServer {

    //private static SensorData sensorData = new SensorData();

    public static void main(String[] args) throws Exception {
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/write", new WriteHandler());
//        server.createContext("/read", new ReadHandler());
        server.setExecutor(null); // creates a default executor
        server.start();
    }

    static class WriteHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange t) throws IOException {

            InputStream inputStream = t.getRequestBody();
            BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
            String line;
            Gson gson = new Gson();
            while ((line = reader.readLine()) != null) {
                System.out.println(line);
                //sensorData = gson.fromJson(line, SensorData.class);
            }

            // Response
            String response = "{code='OK', status=200}\n";
            t.sendResponseHeaders(200, response.length());
            OutputStream os = t.getResponseBody();
            os.write(response.getBytes());
            os.close();
        }
    }
//
//    static class ReadHandler implements HttpHandler {
//        @Override
//        public void handle(HttpExchange t) throws IOException {
//
//            // Response
//            Gson gson = new Gson();
//            String response = gson.toJson(sensorData);
//            t.sendResponseHeaders(200, response.length());
//            OutputStream os = t.getResponseBody();
//            os.write(response.getBytes());
//            os.close();
//        }
//    }
}
