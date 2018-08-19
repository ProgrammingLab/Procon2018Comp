import socket

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('localhost', 54215))
server_socket.listen(1)
while True:
    (client_socket, address) = server_socket.accept()
    print(address)
