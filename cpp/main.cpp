#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <regex>
#include <sstream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

std::string numberToSpanish(int n) {
    std::vector<std::string> unidades = {"cero", "uno", "dos", "tres", "cuatro", 
                                          "cinco", "seis", "siete", "ocho", "nueve"};
    std::vector<std::string> especiales = {"diez", "once", "doce", "trece", "catorce", 
                                            "quince", "dieciséis", "diecisiete", "dieciocho", "diecinueve"};
    std::vector<std::string> decenas = {"", "diez", "veinte", "treinta", "cuarenta",
                                         "cincuenta", "sesenta", "setenta", "ochenta", "noventa"};
    std::vector<std::string> centenas = {"", "cien", "doscientos", "trescientos", "cuatrocientos",
                                          "quinientos", "seiscientos", "setecientos", "ochocientos", "novecientos"};

    if (n == 0) {
        return "cero";
    }

    if (n < 10) {
        return unidades[n];
    }

    if (n < 20) {
        return especiales[n - 10];
    }

    if (n < 30) {
        if (n == 20) {
            return "veinte";
        }
        return "veinti" + unidades[n - 20];
    }

    if (n < 100) {
        int dec = n / 10;
        int uni = n % 10;
        if (uni == 0) {
            return decenas[dec];
        }
        return decenas[dec] + " y " + unidades[uni];
    }

    if (n < 1000) {
        int cent = n / 100;
        int resto = n % 100;
        if (cent == 1 && resto == 0) {
            return "cien";
        }
        if (cent == 1) {
            return "ciento " + numberToSpanish(resto);
        }
        if (resto == 0) {
            return centenas[cent];
        }
        return centenas[cent] + " " + numberToSpanish(resto);
    }

    if (n < 10000) {
        int miles = n / 1000;
        int resto = n % 1000;
        if (miles == 1) {
            if (resto == 0) {
                return "mil";
            }
            return "mil " + numberToSpanish(resto);
        }
        if (resto == 0) {
            return numberToSpanish(miles) + " mil";
        }
        return numberToSpanish(miles) + " mil " + numberToSpanish(resto);
    }

    return "número fuera de rango (máximo 9999)";
}

std::string obtenerParametro(const std::string& request) {
    std::regex pattern("GET /\\?n=([^ ]*)");
    std::smatch match;
    if (std::regex_search(request, match, pattern)) {
        return match[1].str();
    }
    return "";
}

void enviarRespuesta(SOCKET clientSocket, const std::string& body) {
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/plain; charset=utf-8\r\n";
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += body;
    send(clientSocket, response.c_str(), response.length(), 0);
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error al crear socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error al bindear" << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    listen(listenSocket, SOMAXCONN);

    std::cout << "Servidor iniciado en http://localhost:8080" << std::endl;
    std::cout << "Ejemplo: http://localhost:8080/?n=10" << std::endl;
    std::cout << "VERSIÓN 3: Conversión nativa a español (código base del lenguaje)" << std::endl;
    std::cout << "Esperando peticiones..." << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            continue;
        }
        char buffer[4096];
        int bytesRecibidos = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRecibidos > 0) {
            buffer[bytesRecibidos] = '\0';
            std::string request(buffer);

            std::string numStr = obtenerParametro(request);

            if (numStr.empty()) {
                enviarRespuesta(clientSocket, "Usa: ?n=10 (número entre 0 y 9999)");
                continue;
            }

            try {
                int num = std::stoi(numStr);

                if (num < 0 || num > 9999) {
                    enviarRespuesta(clientSocket, "Error: Número fuera de rango (0-9999)");
                    continue;
                }

                std::string resultado = numberToSpanish(num);

                std::cout << "Número: " << num << " → " << resultado << std::endl;

                enviarRespuesta(clientSocket, resultado);

            } catch (const std::exception& e) {
                enviarRespuesta(clientSocket, "Error: El parámetro debe ser un número");
            }
        }

        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}