use strict;
use warnings;
use HTTP::Daemon;
use HTTP::Status;
use LWP::UserAgent;
use WWW::Google::Translate;
use JSON;

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
            my $soap_request = qq{<?xml version="1.0" encoding="utf-8"?>
<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
  <soap:Body>
    <NumberToWords xmlns="http://www.dataaccess.com/webservicesserver/">
      <ubiNum>$num</ubiNum>
    </NumberToWords>
  </soap:Body>
</soap:Envelope>};
 
            my $ua = LWP::UserAgent->new;
            $ua->timeout(60);
            $ua->ssl_opts( verify_hostname => 0 );
            
            my $response = $ua->post(
                'https://www.dataaccess.com/webservicesserver/NumberConversion.wso',
                'Content-Type' => 'text/xml; charset=utf-8',
                'SOAPAction' => 'http://www.dataaccess.com/webservicesserver/NumberToWords',
                'Content' => $soap_request
            );
 
            my $resultado;
            if ($response->is_success) {
                my $content = $response->content;
                if ($content =~ /<[^>]*NumberToWordsResult[^>]*>(.*?)<\/[^>]*NumberToWordsResult>/) {
                    my $resultado_ingles = $1;
                    $resultado_ingles =~ s/^\s+|\s+$//g;
                    
                    eval {
                        my $trans = WWW::Google::Translate->new();
                        $resultado = $trans->translate(
                            source => 'en',
                            target => 'es',
                            text => $resultado_ingles
                        );
                    };
                    if ($@) {
                        $resultado = traducir_con_api_directa($resultado_ingles);
                    }
                } else {
                    $resultado = "No se encontró resultado";
                }
            } else {
                $resultado = "Error HTTP: " . $response->status_line;
            }
 
            $connection->send_response(HTTP::Response->new(200, 'OK', ['Content-Type' => 'text/plain'], $resultado));
        } else {
            $connection->send_response(HTTP::Response->new(200, 'OK', ['Content-Type' => 'text/plain'], "Usa: ?n=10"));
        }
    }
    $connection->close;
    undef($connection);
}

sub traducir_con_api_directa {
    my ($texto) = @_;
 
    my $texto_escaped = $texto;
    $texto_escaped =~ s/ /%20/g;
    $texto_escaped =~ s/"/%22/g;
 
    my $url = "https://translate.googleapis.com/translate_a/single?client=gtx&sl=en&tl=es&dt=t&q=$texto_escaped";
 
    my $ua = LWP::UserAgent->new;
    $ua->timeout(10);
    $ua->ssl_opts( verify_hostname => 0 );
    
    my $response = $ua->get($url);
 
    if ($response->is_success) {
        my $data = decode_json($response->content);
        if ($data && ref($data) eq 'ARRAY' && $data->[0]) {
            my $traduccion = '';
            foreach my $item (@{$data->[0]}) {
                $traduccion .= $item->[0];
            }
            return $traduccion;
        }
    }
    return $texto;  
}