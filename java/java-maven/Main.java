
import com.sun.net.httpserver.HttpServer;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpExchange;

import java.net.InetSocketAddress;
import java.io.OutputStream;

public class Main {
    public static void main(String[] args) throws Exception {
        System.out.println("Servidor iniciado en http://localhost:8080");
        System.out.println("Ejemplo: http://localhost:8080/?n=10");

        HttpServer server = HttpServer.create(new InetSocketAddress(8080), 0);
        server.createContext("/", new NumberHandler());
        server.setExecutor(null);
        server.start();
    }

    static class NumberHandler implements HttpHandler {
        @Override
        public void handle(HttpExchange exchange) throws java.io.IOException {
            try {
                String query = exchange.getRequestURI().getQuery();
                String numStr = null;

                if (query != null) {
                    String[] params = query.split("&");
                    for (String param : params) {
                        if (param.startsWith("n=")) {
                            numStr = param.substring(2);
                            break;
                        }
                    }
                }

                if (numStr == null || numStr.isEmpty()) {
                    String response = "Usa: ?n=10 (número entre 0 y 9999)";
                    exchange.sendResponseHeaders(200, response.getBytes().length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                    return;
                }

                try {
                    int num = Integer.parseInt(numStr);

                    if (num < 0 || num > 9999) {
                        String response = "Error: Número fuera de rango (0-9999)";
                        exchange.sendResponseHeaders(200, response.getBytes().length);
                        OutputStream os = exchange.getResponseBody();
                        os.write(response.getBytes());
                        os.close();
                        return;
                    }

                    String resultado = numberToSpanish(num);

                    exchange.sendResponseHeaders(200, resultado.getBytes().length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(resultado.getBytes());
                    os.close();

                } catch (NumberFormatException e) {
                    String response = "Error: El parámetro debe ser un número";
                    exchange.sendResponseHeaders(200, response.getBytes().length);
                    OutputStream os = exchange.getResponseBody();
                    os.write(response.getBytes());
                    os.close();
                }

            } catch (Exception e) {
                e.printStackTrace();
                exchange.sendResponseHeaders(500, 0);
                exchange.close();
            }
        }
    }

    private static String numberToSpanish(int n) {
        String[] unidades = {"cero", "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve"};
        String[] especiales = {"diez", "once", "doce", "trece", "catorce", "quince", 
                               "dieciséis", "diecisiete", "dieciocho", "diecinueve"};
        String[] decenas = {"", "diez", "veinte", "treinta", "cuarenta", "cincuenta",
                            "sesenta", "setenta", "ochenta", "noventa"};
        String[] centenas = {"", "cien", "doscientos", "trescientos", "cuatrocientos",
                             "quinientos", "seiscientos", "setecientos", "ochocientos", "novecientos"};

        if (n == 0) return "cero";

        if (n < 10) return unidades[n];

        if (n < 20) return especiales[n - 10];

        if (n < 30) {
            if (n == 20) return "veinte";
            return "veinti" + unidades[n - 20];
        }

        if (n < 100) {
            int dec = n / 10;
            int uni = n % 10;
            if (uni == 0) return decenas[dec];
            return decenas[dec] + " y " + unidades[uni];
        }

        if (n < 1000) {
            int cent = n / 100;
            int resto = n % 100;
            if (cent == 1 && resto == 0) return "cien";
            if (cent == 1) return "ciento " + numberToSpanish(resto);
            if (resto == 0) return centenas[cent];
            return centenas[cent] + " " + numberToSpanish(resto);
        }

        if (n < 10000) {
            int miles = n / 1000;
            int resto = n % 1000;
            if (miles == 1) {
                if (resto == 0) return "mil";
                return "mil " + numberToSpanish(resto);
            }
            if (resto == 0) return numberToSpanish(miles) + " mil";
            return numberToSpanish(miles) + " mil " + numberToSpanish(resto);
        }

        return "número fuera de rango (máximo 9999)";
    }
}