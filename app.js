const http = require('http');
const url = require('url');
const soap = require('soap');
const { translate } = require('@vitalets/google-translate-api');

const server = http.createServer((req, res) => {
    const query = new URLSearchParams(req.url.split('?')[1] || '');
    const num = query.get('n');
    
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
        
        client.NumberToWords({ ubiNum: num }, async (err, result) => {
            if (err) {
                res.writeHead(500, { 'Content-Type': 'text/plain' });
                res.end('Error al llamar al servicio SOAP: ' + err.message);
                return;
            }
            
            try {
                const translation = await translate(result.NumberToWordsResult, { to: 'es' });
                res.writeHead(200, { 'Content-Type': 'text/plain' });
                res.end(translation.text);
            } catch (err) {
                console.error('Error en traducción:', err.message);
                res.writeHead(200, { 'Content-Type': 'text/plain' });
                res.end(result.NumberToWordsResult);
            }
        });
    });
});
server.listen(3000, () => {
    console.log('Servidor iniciado en http://localhost:3000');
});
