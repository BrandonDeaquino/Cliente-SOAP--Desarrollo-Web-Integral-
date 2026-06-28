use std::io::prelude::*;
use std::net::{TcpListener, TcpStream};
use regex::Regex;

fn number_to_spanish(n: i32) -> String {
    let unidades = vec!["cero", "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve"];
    let especiales = vec!["diez", "once", "doce", "trece", "catorce", "quince", 
                          "dieciséis", "diecisiete", "dieciocho", "diecinueve"];
    let decenas = vec!["", "diez", "veinte", "treinta", "cuarenta", "cincuenta",
                       "sesenta", "setenta", "ochenta", "noventa"];
    let centenas = vec!["", "cien", "doscientos", "trescientos", "cuatrocientos",
                        "quinientos", "seiscientos", "setecientos", "ochocientos", "novecientos"];

    if n == 0 {
        return "cero".to_string();
    }

    if n < 10 {
        return unidades[n as usize].to_string();
    }

    if n < 20 {
        return especiales[(n - 10) as usize].to_string();
    }

    if n < 30 {
        if n == 20 {
            return "veinte".to_string();
        }
        return format!("veinti{}", unidades[(n - 20) as usize]);
    }

    if n < 100 {
        let dec = n / 10;
        let uni = n % 10;
        if uni == 0 {
            return decenas[dec as usize].to_string();
        }
        return format!("{} y {}", decenas[dec as usize], unidades[uni as usize]);
    }

    if n < 1000 {
        let cent = n / 100;
        let resto = n % 100;
        if cent == 1 && resto == 0 {
            return "cien".to_string();
        }
        if cent == 1 {
            return format!("ciento {}", number_to_spanish(resto));
        }
        if resto == 0 {
            return centenas[cent as usize].to_string();
        }
        return format!("{} {}", centenas[cent as usize], number_to_spanish(resto));
    }

    if n < 10000 {
        let miles = n / 1000;
        let resto = n % 1000;
        if miles == 1 {
            if resto == 0 {
                return "mil".to_string();
            }
            return format!("mil {}", number_to_spanish(resto));
        }
        if resto == 0 {
            return format!("{} mil", number_to_spanish(miles));
        }
        return format!("{} mil {}", number_to_spanish(miles), number_to_spanish(resto));
    }

    "número fuera de rango (máximo 9999)".to_string()
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
    println!("VERSIÓN 3: Conversión nativa a español (código base del lenguaje)");
    println!("Esperando peticiones...");

    for stream in listener.incoming() {
        match stream {
            Ok(mut stream) => {
                let mut buffer = [0; 4096];
                match stream.read(&mut buffer) {
                    Ok(size) => {
                        let request = String::from_utf8_lossy(&buffer[..size]);
                        let num_str = obtener_parametro(&request);

                        if let Some(num_str) = num_str {
                            if let Ok(num) = num_str.parse::<i32>() {
                                if num < 0 || num > 9999 {
                                    enviar_respuesta(stream, "Error: Número fuera de rango (0-9999)");
                                } else {
                                    let resultado = number_to_spanish(num);
                                    enviar_respuesta(stream, &resultado);
                                }
                            } else {
                                enviar_respuesta(stream, "Error: El parámetro debe ser un número");
                            }
                        } else {
                            enviar_respuesta(stream, "Usa: ?n=10 (número entre 0 y 9999)");
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