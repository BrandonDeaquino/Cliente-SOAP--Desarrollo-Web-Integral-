package main

import (
	"encoding/json"
	"encoding/xml"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
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
	resultadoEspanol := traducirConGoogle(response.Result)
	
	w.Write([]byte(resultadoEspanol))
}

func traducirConGoogle(texto string) string {
	url := fmt.Sprintf("https://translate.googleapis.com/translate_a/single?client=gtx&sl=en&tl=es&dt=t&q=%s", url.QueryEscape(texto))
	
	resp, err := http.Get(url)
	if err != nil {
		return texto
	}
	defer resp.Body.Close()
	
	body, _ := ioutil.ReadAll(resp.Body)
	
	var data []interface{}
	err = json.Unmarshal(body, &data)
	if err != nil {
		return texto
	}
	if len(data) > 0 {
		firstArray, ok := data[0].([]interface{})
		if ok && len(firstArray) > 0 {
			var traduccion string
			for _, item := range firstArray {
				itemArray, ok := item.([]interface{})
				if ok && len(itemArray) > 0 {
					if str, ok := itemArray[0].(string); ok {
						traduccion += str
					}
				}
			}
			if traduccion != "" {
				return traduccion
			}
		}
	}
	
	return texto
}

func parseInt(s string) int {
	var i int
	fmt.Sscanf(s, "%d", &i)
	return i
}