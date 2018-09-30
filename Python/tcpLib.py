# coding: utf-8

import socket

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

def mysend(socket, msg):
    totalsent = 0
    while totalsent < len(msg):
        sent = socket.send(msg[totalsent:])
        if sent == 0:
            raise RuntimeError("connection broken")
        totalsent = totalsent + sent
    # print('totalsent: ' + str(totalsent))

# 先頭に10バイトの長さ情報を含んだプロトコル
def mysend_with_sign(sock, msg):
    sign = bytearray(str(len(msg)).encode('ascii'))
    while len(sign) < 10:
        sign.extend(' '.encode('ascii'))
    mysend(sock, sign)
    mysend(sock, msg)

# 先頭に10バイトの長さ情報を含んだプロトコル
def myreceive_with_sign(sock):
    size = int(myreceive(sock, 10).decode('ascii'))
    return myreceive(sock, size)
