import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.net.InetSocketAddress;
import java.net.URI;
import java.net.http.HttpClient;

import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.io.OutputStream;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class Main {
    private static final String WSDL_URL = "https://www.dataaccess.com/webservicesserver/NumberConversion.wso?WSDL";
    
    public static void main(String[] args) throws Exception {
        System.out.println("Servidor iniciado en http://localhost:8080");
        System.out.println("Ejemplo: http://localhost:8080/?n=10");
        System.out.println("Usando WSDL: " + WSDL_URL);
        
        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new SoapHandler());
        server.setExecutor(null);
        server.start();
    }
    
    static class SoapHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws java.io.IOException {
            try {
                String query = exchange.getRequestURI().getQuery();
                String num = null;
                
                if (query != null) {
                    String[] params = query.split("&");
                    for (String param : params) {
                        if (param.startsWith("n=")) {
                            num = param.substring(2);
                            break;
                        }
                    }
                }
                
                if (num == null || num.isEmpty()) {
                    String response = "Usa: ?n=10";
                    exchange.sendResponseHeaders(200, response.getBytes().length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                    return;
                }
                
                try {
                    String soapRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" +
                        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">" +
                        "  <soap:Body>" +
                        "    <NumberToWords xmlns=\"http://www.dataaccess.com/webservicesserver/\">" +
                        "      <ubiNum>" + num + "</ubiNum>" +
                        "    </NumberToWords>" +
                        "  </soap:Body>" +
                        "</soap:Envelope>";
                    
                    HttpClient client = HttpClient.newHttpClient();
                    HttpRequest request = HttpRequest.newBuilder()
                        .uri(URI.create("https://www.dataaccess.com/webservicesserver/NumberConversion.wso"))
                        .header("Content-Type", "text/xml; charset=utf-8")
                        .header("SOAPAction", "\"http://www.dataaccess.com/webservicesserver/NumberToWords\"")
                        .POST(HttpRequest.BodyPublishers.ofString(soapRequest))
                        .build();
                    
                    HttpResponse<String> response = client.send(request, HttpResponse.BodyHandlers.ofString());

                    Pattern pattern = Pattern.compile("<[^>]*NumberToWordsResult[^>]*>(.*?)</[^>]*NumberToWordsResult>", Pattern.DOTALL);
                    Matcher matcher = pattern.matcher(response.body());
                    
                    String result = matcher.find() ? matcher.group(1).trim() : "No se encontró resultado";
                    
                    exchange.sendResponseHeaders(200, result.getBytes().length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(result.getBytes());
                    os.close();
                    
                } catch (Exception e) {
                    String error = "Error: " + e.getMessage();
                    exchange.sendResponseHeaders(500, error.getBytes().length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(error.getBytes());
                    os.close();
                }
                
            } catch (Exception e) {
                e.printStackTrace();
                exchange.sendResponseHeaders(500, 0);
                exchange.close();
            }
        }
    }
}