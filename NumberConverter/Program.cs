using System;
using System.Globalization;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using Humanizer;

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
    
    static void ProcessRequest(HttpListenerContext context)
    {
        var response = context.Response;
        var query = context.Request.QueryString;
        var num = query["n"];
        
        if (string.IsNullOrEmpty(num))
        {
            byte[] bufferData = Encoding.UTF8.GetBytes("Usa: ?n=10 (número entre 0 y 9999)");
            response.ContentLength64 = bufferData.Length;
            response.OutputStream.Write(bufferData, 0, bufferData.Length);
            response.Close();
            return;
        }
        
        try
        {
            if (!int.TryParse(num, out int number))
            {
                byte[] bufferError = Encoding.UTF8.GetBytes("Error: El parámetro debe ser un número");
                response.ContentLength64 = bufferError.Length;
                response.OutputStream.Write(bufferError, 0, bufferError.Length);
                response.Close();
                return;
            }
            
            if (number < 0 || number > 9999)
            {
                byte[] bufferRange = Encoding.UTF8.GetBytes("Error: Número fuera de rango (0-9999)");
                response.ContentLength64 = bufferRange.Length;
                response.OutputStream.Write(bufferRange, 0, bufferRange.Length);
                response.Close();
                return;
            }
            
            var culture = new CultureInfo("es-ES");
            var resultado = number.ToWords(culture);
            
            byte[] bufferResult = Encoding.UTF8.GetBytes(resultado);
            response.ContentLength64 = bufferResult.Length;
            response.OutputStream.Write(bufferResult, 0, bufferResult.Length);
            response.Close();
        }
        catch (Exception ex)
        {
            byte[] bufferEx = Encoding.UTF8.GetBytes($"Error: {ex.Message}");
            response.ContentLength64 = bufferEx.Length;
            response.OutputStream.Write(bufferEx, 0, bufferEx.Length);
            response.Close();
        }
    }
}