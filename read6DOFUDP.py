import socket
from pymavlink import fgFDM


import socket

UDP_IP = "127.0.0.1"
UDP_PORT = 5503

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))
fdm = fgFDM.fgFDM()

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    fdm.parse(data)
    print fdm.values
    print "received message:", data

