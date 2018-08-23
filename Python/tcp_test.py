import socket
import json

def myreceive(socket, byte_size):
    chunks = []
    cnt = byte_size
    while cnt > 0:
        chunk = socket.recv(cnt)
        if chunk == b'':
            raise RuntimeError("connection broken")
        chunks.append(chunk)
        cnt -= len(chunk)
    return b''.join(chunks)

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('localhost', 54215))
server_socket.listen(1)
while True:
    (client_socket, address) = server_socket.accept()
    size = int(myreceive(client_socket, 10).decode('ascii'))
    print(size)
    body = json.loads(myreceive(client_socket, size).decode('utf-8'))
    print(body)
