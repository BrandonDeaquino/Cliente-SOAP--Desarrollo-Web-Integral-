using System;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Net.Http;

class Program
{
    static async Task Main(string[] args)
    {
        var listener = new HttpListener();
        listener.Prefixes.Add("http://localhost:5000/");
        listener.Start();
        Console.WriteLine("Servidor iniciado en http://localhost:5000");
        
        while (true)
        {
            var context = await listener.GetContextAsync();
            _ = Task.Run(() => ProcessRequest(context));
        }
    }
    
    static async Task ProcessRequest(HttpListenerContext context)
    {
        var query = context.Request.QueryString;
        var num = query["n"];
        
        if (string.IsNullOrEmpty(num))
        {
            var response = context.Response;
            var buffer = Encoding.UTF8.GetBytes("Usa: ?n=10");
            response.ContentLength64 = buffer.Length;
            await response.OutputStream.WriteAsync(buffer, 0, buffer.Length);
            response.Close();
            return;
        }
        
        try
        {
            var result = await ConsumirSoapAsync(int.Parse(num));
            
            var response = context.Response;
            var buffer = Encoding.UTF8.GetBytes(result);
            response.ContentLength64 = buffer.Length;
            await response.OutputStream.WriteAsync(buffer, 0, buffer.Length);
            response.Close();
        }
        catch (Exception ex)
        {
            var response = context.Response;
            var buffer = Encoding.UTF8.GetBytes($"Error: {ex.Message}");
            response.ContentLength64 = buffer.Length;
            await response.OutputStream.WriteAsync(buffer, 0, buffer.Length);
            response.Close();
        }
    }
    
    static async Task<string> ConsumirSoapAsync(int num)
    {
        string soapRequest = $@"<?xml version=""1.0"" encoding=""utf-8""?>
<soap:Envelope xmlns:soap=""http://schemas.xmlsoap.org/soap/envelope/"">
  <soap:Body>
    <NumberToWords xmlns=""http://www.dataaccess.com/webservicesserver/"">
      <ubiNum>{num}</ubiNum>
    </NumberToWords>
  </soap:Body>
</soap:Envelope>";
        
        using var client = new HttpClient();
        client.Timeout = TimeSpan.FromSeconds(60);
        
        var content = new StringContent(soapRequest, Encoding.UTF8, "text/xml");
        content.Headers.Add("SOAPAction", "\"http://www.dataaccess.com/webservicesserver/NumberToWords\"");
        
        var response = await client.PostAsync(
            "https://www.dataaccess.com/webservicesserver/NumberConversion.wso",
            content
        );
        
        var responseBody = await response.Content.ReadAsStringAsync();
        var match = System.Text.RegularExpressions.Regex.Match(
            responseBody,
            @"<[^>]*NumberToWordsResult[^>]*>(.*?)</[^>]*NumberToWordsResult>",
            System.Text.RegularExpressions.RegexOptions.Singleline
        );
        
        if (match.Success)
        {
            return match.Groups[1].Value.Trim();
        }
        
        return "No se encontró resultado";
    }
}