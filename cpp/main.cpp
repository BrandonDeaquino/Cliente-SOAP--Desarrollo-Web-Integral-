#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <regex>
#include <sstream>
#include <curl/curl.h>

#pragma comment(lib, "ws2_32.lib")

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total = size * nmemb;
    output->append((char*)contents, total);
    return total;
}

std::string hacerPeticionSOAP(const std::string& num) {
    std::string soapRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" 
        "<soap:Envelope xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        "  <soap:Body>"
        "    <NumberToWords xmlns=\"http://www.dataaccess.com/webservicesserver/\">"
        "      <ubiNum>" + num + "</ubiNum>"
        "    </NumberToWords>"
        "  </soap:Body>"
        "</soap:Envelope>";

    CURL* curl = curl_easy_init();
    if (!curl) {
        return "Error al inicializar curl";
    }

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: text/xml; charset=utf-8");
    headers = curl_slist_append(headers, "SOAPAction: \"http://www.dataaccess.com/webservicesserver/NumberToWords\"");

    curl_easy_setopt(curl, CURLOPT_URL, "https://www.dataaccess.com/webservicesserver/NumberConversion.wso");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, soapRequest.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, soapRequest.length());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        return "Error en petición SOAP: " + std::string(curl_easy_strerror(res));
    }

    return response;
}

std::string extraerResultado(const std::string& xml) {
    std::regex pattern("<[^>]*NumberToWordsResult[^>]*>(.*?)</[^>]*NumberToWordsResult>");
    std::smatch match;
    if (std::regex_search(xml, match, pattern)) {
        return match[1].str();
    }
    return "No se encontró resultado";
}

std::string traducirConGoogle(const std::string& texto) {
    if (texto.empty() || texto.find("No se encontró") != std::string::npos) {
        return texto;
    }

    CURL* curl = curl_easy_init();
    if (!curl) return texto;

    char* textoCodificado = curl_easy_escape(curl, texto.c_str(), texto.length());
    std::string url = "https://translate.googleapis.com/translate_a/single?client=gtx&sl=en&tl=es&dt=t&q=" + std::string(textoCodificado);
    curl_free(textoCodificado);

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "Error en traducción: " << curl_easy_strerror(res) << std::endl;
        return texto;
    }

    std::regex pattern("\\[\\[\\[\"([^\"]+)\"");
    std::smatch match;
    if (std::regex_search(response, match, pattern)) {
        return match[1].str();
    }

    return texto;
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
    curl_global_init(CURL_GLOBAL_DEFAULT);

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
    std::cout << "Usando WSDL: https://www.dataaccess.com/webservicesserver/NumberConversion.wso?WSDL" << std::endl;
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

            std::string num = obtenerParametro(request);

            if (num.empty()) {
                enviarRespuesta(clientSocket, "Usa: ?n=10");
                continue;
            }

            std::string response = hacerPeticionSOAP(num);
            std::string resultadoIngles = extraerResultado(response);

            std::cout << "Resultado en inglés: " << resultadoIngles << std::endl;

            std::string resultadoEspanol = traducirConGoogle(resultadoIngles);

            std::cout << "Traducción a español: " << resultadoEspanol << std::endl;

            enviarRespuesta(clientSocket, resultadoEspanol);
        } else {
            closesocket(clientSocket);
        }
    }

    closesocket(listenSocket);
    WSACleanup();
    curl_global_cleanup();
    return 0;
}