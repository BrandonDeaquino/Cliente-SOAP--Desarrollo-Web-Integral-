require 'sinatra'
require 'humanize'

get '/' do
  num = params['n'].to_i
  num.humanize(locale: :es)
end