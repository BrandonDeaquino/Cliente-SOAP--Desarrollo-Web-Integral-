use std::io::prelude::*;
use std::net::{TcpListener, TcpStream};
use regex::Regex;

fn extraer_resultado(xml: &str) -> String {
    let re = Regex::new(r"<[^>]*NumberToWordsResult[^>]*>(.*?)</[^>]*NumberToWordsResult>").unwrap();
    if let Some(cap) = re.captures(xml) {
        return cap[1].to_string();
    }
    "No se encontró resultado".to_string()
}

fn hacer_peticion_soap(num: &str) -> String {
    let soap_request = format!(
        r#"<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
  <soap:Body>
    <NumberToWords xmlns="http://www.dataaccess.com/webservicesserver/">
      <ubiNum>{}</ubiNum>
    </NumberToWords>
  </soap:Body>
</soap:Envelope>"#,
        num
    );

    let client = reqwest::blocking::Client::new();
    let response = client
        .post("https://www.dataaccess.com/webservicesserver/NumberConversion.wso")
        .header("Content-Type", "text/xml; charset=utf-8")
        .header("SOAPAction", "\"http://www.dataaccess.com/webservicesserver/NumberToWords\"")
        .body(soap_request)
        .send();

    match response {
        Ok(resp) => {
            if let Ok(body) = resp.text() {
                return body;
            }
            "Error al leer respuesta".to_string()
        }
        Err(e) => format!("Error en petición SOAP: {}", e),
    }
}

fn obtener_parametro(request: &str) -> Option<String> {
    let re = Regex::new(r"GET /\?n=([^ ]*)").unwrap();
    if let Some(cap) = re.captures(request) {
        return Some(cap[1].to_string());
    }
    None
}

fn enviar_respuesta(mut stream: TcpStream, body: &str) {
    let response = format!(
        "HTTP/1.1 200 OK\r\n\
         Content-Type: text/plain; charset=utf-8\r\n\
         Content-Length: {}\r\n\
         Connection: close\r\n\r\n\
         {}",
        body.len(),
        body
    );
    let _ = stream.write(response.as_bytes());
    let _ = stream.flush();
}

fn main() {
    let listener = TcpListener::bind("127.0.0.1:8080").expect("Error al bindear");
    println!("Servidor iniciado en http://localhost:8080");
    println!("Ejemplo: http://localhost:8080/?n=10");
    println!("Usando WSDL: https://www.dataaccess.com/webservicesserver/NumberConversion.wso?WSDL");
    println!("Esperando peticiones...");

    for stream in listener.incoming() {
        match stream {
            Ok(mut stream) => {
                let mut buffer = [0; 4096];
                match stream.read(&mut buffer) {
                    Ok(size) => {
                        let request = String::from_utf8_lossy(&buffer[..size]);
                        let num = obtener_parametro(&request);

                        if let Some(num) = num {
                            let response = hacer_peticion_soap(&num);
                            let resultado = extraer_resultado(&response);
                            enviar_respuesta(stream, &resultado);
                        } else {
                            enviar_respuesta(stream, "Usa: ?n=10");
                        }
                    }
                    Err(e) => {
                        eprintln!("Error al leer petición: {}", e);
                    }
                }
            }
            Err(e) => {
                eprintln!("Error al aceptar conexión: {}", e);
            }
        }
    }
}