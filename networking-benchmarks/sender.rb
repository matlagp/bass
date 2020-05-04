#!/usr/bin/env ruby

require 'socket'

PORT = 2137

address, mode = ARGV

unless ARGV.length >= 2 && ['udp', 'tcp'].include?(mode)
    puts "usage: ./sender.rb <ip> (udp|tcp)"
    exit -1
end

if mode == 'udp'
    s = UDPSocket.new

    loop do
        s.send('x' * 1400, 0, address, PORT)
    end
else
    s = TCPSocket.open(address, PORT)

    loop do
        s.puts('x' * 1400)
    end
end
