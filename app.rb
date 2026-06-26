require 'sinatra'

get '/' do
  num = params['n'].to_i
  
  soap_request = <<~SOAP
    <?xml version="1.0" encoding="utf-8"?>
    <soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
      <soap:Body>
        <NumberToWords xmlns="http://www.dataaccess.com/webservicesserver/">
          <ubiNum>#{num}</ubiNum>
        </NumberToWords>
      </soap:Body>
    </soap:Envelope>
  SOAP
  
  File.write("soap_request.xml", soap_request)
  
  curl_cmd = 'curl -s -X POST "https://www.dataaccess.com/webservicesserver/NumberConversion.wso" -H "Content-Type: text/xml; charset=utf-8" -H "SOAPAction: http://www.dataaccess.com/webservicesserver/NumberToWords" -d "@soap_request.xml" --insecure --connect-timeout 60 --max-time 120'
  
  response = `#{curl_cmd}`
  File.delete("soap_request.xml") if File.exist?("soap_request.xml")
  
  if response && !response.empty?
    match = response.match(/<[^>]*NumberToWordsResult[^>]*>(.*?)<\/[^>]*NumberToWordsResult>/m)
    if match
      return match[1].strip
    else
      return "No se encontró resultado"
    end
  else
    return "Error: No se recibió respuesta"
  end
end
