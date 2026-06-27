package main

import (
	"fmt"
	"net/http"
	"strconv"
)

func numberToSpanish(n int) string {
	unidades := []string{"cero", "uno", "dos", "tres", "cuatro", "cinco", "seis", "siete", "ocho", "nueve"}
	especiales := []string{"diez", "once", "doce", "trece", "catorce", "quince", "dieciséis", "diecisiete", "dieciocho", "diecinueve"}
	decenas := []string{"", "diez", "veinte", "treinta", "cuarenta", "cincuenta", "sesenta", "setenta", "ochenta", "noventa"}
	centenas := []string{"", "cien", "doscientos", "trescientos", "cuatrocientos", "quinientos", "seiscientos", "setecientos", "ochocientos", "novecientos"}
	if n == 0 {
		return "cero"
	}

	if n < 10 {
		return unidades[n]
	}

	if n < 20 {
		return especiales[n-10]
	}

	if n < 30 {
		if n == 20 {
			return "veinte"
		}
		return "veinti" + unidades[n-20]
	}

	if n < 100 {
		dec := n / 10
		uni := n % 10
		if uni == 0 {
			return decenas[dec]
		}
		return decenas[dec] + " y " + unidades[uni]
	}
	if n < 1000 {
		cent := n / 100
		resto := n % 100
		if cent == 1 && resto == 0 {
			return "cien"
		}
		if cent == 1 {
			return "ciento " + numberToSpanish(resto)
		}
		if resto == 0 {
			return centenas[cent]
		}
		return centenas[cent] + " " + numberToSpanish(resto)
	}
	if n < 10000 {
		miles := n / 1000
		resto := n % 1000
		if miles == 1 {
			if resto == 0 {
				return "mil"
			}
			return "mil " + numberToSpanish(resto)
		}
		if resto == 0 {
			return numberToSpanish(miles) + " mil"
		}
		return numberToSpanish(miles) + " mil " + numberToSpanish(resto)
	}

	return "número fuera de rango (máximo 9999)"
}

func handler(w http.ResponseWriter, r *http.Request) {
	numStr := r.URL.Query().Get("n")
	if numStr == "" {
		w.Write([]byte("Usa: ?n=10 (número entre 0 y 9999)"))
		return
	}
	num, err := strconv.Atoi(numStr)
	if err != nil {
		w.Write([]byte("Error: El parámetro debe ser un número"))
		return
	}
	if num < 0 || num > 9999 {
		w.Write([]byte("Error: Número fuera de rango (0-9999)"))
		return
	}
	resultado := numberToSpanish(num)
	w.Write([]byte(resultado))
}
func main() {
	http.HandleFunc("/", handler)
	fmt.Println("Servidor iniciado en http://localhost:8080")
	fmt.Println("Ejemplo: http://localhost:8080/?n=10")
	http.ListenAndServe(":8080", nil)
}