require 'sinatra'
require 'net/http'
require 'uri'
require 'json'

def traducir_con_google(texto)
  return texto if texto.nil? || texto.empty?
  
  begin
    #USANDO LA API DE GOOGLE TRANSLATE
    url = "https://translate.googleapis.com/translate_a/single?client=gtx&sl=en&tl=es&dt=t&q=#{URI.encode_www_form_component(texto)}"
    
    uri = URI.parse(url)
    http = Net::HTTP.new(uri.host, uri.port)
    http.use_ssl = true
    http.verify_mode = OpenSSL::SSL::VERIFY_NONE
    
    request = Net::HTTP::Get.new(uri.request_uri)
    response = http.request(request)
    
    if response.code == "200"
      data = JSON.parse(response.body)
      if data && data[0]
        traduccion = data[0].map { |item| item[0] }.join
        return traduccion
      end
    end
    return texto
  rescue => e
    puts "Error en traducción: #{e.message}"
    return texto
  end
end

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
      resultado_ingles = match[1].strip
      return traducir_con_google(resultado_ingles)
    else
      return "No se encontró resultado"
    end
  else
    return "Error: No se recibió respuesta"
  end
end
