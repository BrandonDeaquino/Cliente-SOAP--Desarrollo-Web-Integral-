package main

import (
	"encoding/xml"
	"fmt"
	"net/http"
	"github.com/hooklift/gowsdl/soap"
)

type NumberToWords struct {
	XMLName xml.Name `xml:"http://www.dataaccess.com/webservicesserver/ NumberToWords"`
	UbiNum  int      `xml:"ubiNum"`
}

type NumberToWordsResponse struct {
	XMLName xml.Name `xml:"http://www.dataaccess.com/webservicesserver/ NumberToWordsResponse"`
	Result  string   `xml:"NumberToWordsResult"`
}

func main() {
	http.HandleFunc("/", handler)
	fmt.Println("Servidor iniciado en http://localhost:8080")
	http.ListenAndServe(":8080", nil)
}

func handler(w http.ResponseWriter, r *http.Request) {
	num := r.URL.Query().Get("n")
	if num == "" {
		w.Write([]byte("Usa: ?n=10"))
		return
	}

	wsdlURL := "https://www.dataaccess.com/webservicesserver/NumberConversion.wso?WSDL"
	
	client := soap.NewClient(wsdlURL)
	
	request := &NumberToWords{
		UbiNum: parseInt(num),
	}
	
	response := &NumberToWordsResponse{}
	
	err := client.Call("http://www.dataaccess.com/webservicesserver/NumberToWords", request, response)
	if err != nil {
		w.Write([]byte(fmt.Sprintf("Error: %s", err.Error())))
		return
	}
	
	w.Write([]byte(response.Result))
}

func parseInt(s string) int {
	var i int
	fmt.Sscanf(s, "%d", &i)
	return i
}
