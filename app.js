const http = require('http');

function numeroALetras(num) {
    if (isNaN(num) || num < 0 || num > 9999) {
        return 'Número inválido (0-9999)';
    }

    if (num === 0) return 'cero';

    const unidades = ['', 'uno', 'dos', 'tres', 'cuatro', 'cinco', 'seis', 'siete', 'ocho', 'nueve'];
    const especiales = ['diez', 'once', 'doce', 'trece', 'catorce', 'quince', 
                        'dieciséis', 'diecisiete', 'dieciocho', 'diecinueve'];
    const decenas = ['', 'diez', 'veinte', 'treinta', 'cuarenta', 'cincuenta',
                     'sesenta', 'setenta', 'ochenta', 'noventa'];
    const centenas = ['', 'cien', 'doscientos', 'trescientos', 'cuatrocientos',
                      'quinientos', 'seiscientos', 'setecientos', 'ochocientos', 'novecientos'];

    let n = parseInt(num);
    let resultado = '';

    if (n >= 1000) {
        const miles = Math.floor(n / 1000);
        n = n % 1000;
        if (miles === 1) {
            resultado += 'mil';
        } else {
            resultado += numeroALetras(miles) + ' mil';
        }
        if (n > 0) resultado += ' ';
    }
    if (n >= 100) {
        const cent = Math.floor(n / 100);
        n = n % 100;
        if (cent === 1 && n === 0) {
            resultado += 'cien';
        } else if (cent === 1) {
            resultado += 'ciento';
        } else {
            resultado += centenas[cent];
        }
        if (n > 0) resultado += ' ';
    }
    if (n >= 10 && n < 20) {
        resultado += especiales[n - 10];
    } else if (n >= 20) {
        const dec = Math.floor(n / 10);
        const uni = n % 10;
        if (uni === 0) {
            resultado += decenas[dec];
        } else {
            if (dec === 2) {
                resultado += 'veinti' + unidades[uni];
            } else {
                resultado += decenas[dec] + ' y ' + unidades[uni];
            }
        }
    } else if (n > 0) {
        resultado += unidades[n];
    }

    return resultado.trim();
}

const server = http.createServer((req, res) => {
    const query = new URLSearchParams(req.url.split('?')[1] || '');
    const num = query.get('n');
    if (!num) {
        res.writeHead(200, { 'Content-Type': 'text/plain' });
        res.end('Usa: ?n=10 (número entre 0 y 9999)');
        return;
    }
    if (isNaN(num)) {
        res.writeHead(200, { 'Content-Type': 'text/plain' });
        res.end('Error: El parámetro debe ser un número');
        return;
    }
    const numEntero = parseInt(num);
    if (numEntero < 0 || numEntero > 9999) {
        res.writeHead(200, { 'Content-Type': 'text/plain' });
        res.end('Error: Número fuera de rango (0-9999)');
        return;
    }

    const resultado = numeroALetras(numEntero);
    
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end(resultado);
});

server.listen(3000, () => {
    console.log('Servidor iniciado en http://localhost:3000');
    console.log('Ejemplo: http://localhost:3000/?n=10');
});
