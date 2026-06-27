const http = require('http');
const url = require('url');
const soap = require('soap');

const server = http.createServer((req, res) => {
    const query = url.parse(req.url, true).query;
    const num = query.n;
    
    if (!num) {
        res.writeHead(200, { 'Content-Type': 'text/plain' });
        res.end('Usa: ?n=10');
        return;
    }
    
    const wsdlUrl = 'https://www.dataaccess.com/webservicesserver/NumberConversion.wso?WSDL';
    
    soap.createClient(wsdlUrl, (err, client) => {
        if (err) {
            res.writeHead(500, { 'Content-Type': 'text/plain' });
            res.end('Error al crear cliente SOAP: ' + err.message);
            return;
        }
        
        client.NumberToWords({ ubiNum: num }, (err, result) => {
            if (err) {
                res.writeHead(500, { 'Content-Type': 'text/plain' });
                res.end('Error al llamar al servicio SOAP: ' + err.message);
                return;
            }
            
            res.writeHead(200, { 'Content-Type': 'text/plain' });
            res.end(result.NumberToWordsResult);
        });
    });
});

server.listen(3000, () => {
    console.log('Servidor iniciado en http://localhost:3000');
});