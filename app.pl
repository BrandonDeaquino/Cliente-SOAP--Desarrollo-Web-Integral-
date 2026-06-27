use strict;
use warnings;
use HTTP::Daemon;
use HTTP::Status;

sub numero_a_letras {
    my ($num) = @_;
    
    return "Número inválido" unless defined $num && $num =~ /^\d+$/;
    
    my $n = int($num);
    return "cero" if $n == 0;
    
        my @unidades = ('', 'uno', 'dos', 'tres', 'cuatro', 'cinco', 'seis', 'siete', 'ocho', 'nueve');
    my @especiales = ('diez', 'once', 'doce', 'trece', 'catorce', 'quince', 
                      'dieciséis', 'diecisiete', 'dieciocho', 'diecinueve');
    my @decenas = ('', 'diez', 'veinte', 'treinta', 'cuarenta', 'cincuenta',
                   'sesenta', 'setenta', 'ochenta', 'noventa');
    my @centenas = ('', 'cien', 'doscientos', 'trescientos', 'cuatrocientos',
                    'quinientos', 'seiscientos', 'setecientos', 'ochocientos', 'novecientos');
    
    if ($n < 10) {
        return $unidades[$n];
    }
    elsif ($n < 20) {
        return $especiales[$n - 10];
    }
    elsif ($n < 30) {
        return $n == 20 ? "veinte" : "veinti" . $unidades[$n - 20];
    }
    elsif ($n < 100) {
        my $dec = int($n / 10);
        my $uni = $n % 10;
        return $uni == 0 ? $decenas[$dec] : $decenas[$dec] . " y " . $unidades[$uni];
    }
    elsif ($n < 1000) {
        my $cent = int($n / 100);
        my $resto = $n % 100;
        if ($cent == 1 && $resto == 0) {
            return "cien";
        } elsif ($cent == 1) {
            return "ciento " . numero_a_letras($resto);
        } elsif ($resto == 0) {
            return $centenas[$cent];
        } else {
            return $centenas[$cent] . " " . numero_a_letras($resto);
        }
    }
    elsif ($n < 10000) {
        my $mil = int($n / 1000);
        my $resto = $n % 1000;
        if ($mil == 1) {
            return $resto == 0 ? "mil" : "mil " . numero_a_letras($resto);
        } else {
            return $resto == 0 ? numero_a_letras($mil) . " mil" : numero_a_letras($mil) . " mil " . numero_a_letras($resto);
        }
    }
    else {
        return "número fuera de rango (máximo 9999)";
    }
}

my $daemon = HTTP::Daemon->new(
    LocalPort => 5000,
    ReuseAddr => 1
) or die "No se pudo iniciar el servidor: $!";

print "Servidor iniciado en http://localhost:5000\n";

while (my $connection = $daemon->accept) {
    while (my $request = $connection->get_request) {
        my $uri = $request->uri;
        my $query = $uri->query || '';
        my %params = map { split('=', $_) } split('&', $query);
        my $num = $params{'n'};
        
        if (defined $num && $num ne '') {
            my $resultado = numero_a_letras($num);
            $connection->send_response(HTTP::Response->new(200, 'OK', ['Content-Type' => 'text/plain'], $resultado));
        } else {
            $connection->send_response(HTTP::Response->new(200, 'OK', ['Content-Type' => 'text/plain'], "Usa: ?n=10"));
        }
    }
    $connection->close;
    undef($connection);
}
