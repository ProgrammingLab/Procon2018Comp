# coding: utf-8

from library import *
import os
import socket

def data_server(output_dir):
    game_id = 0
    while True:
        if not os.path.isdir(output_dir + '/' + str(game_id)):
            break
        game_id += 1

    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('', 54216))
    server_socket.listen(1)
    print('started server')
    while True:
        (client_socket, address) = server_socket.accept()
        receive_size = int(myreceive(client_socket, 10).decode('ascii'))
        # print(size)
        receive_body = json.loads(myreceive(client_socket, receive_size).decode('utf-8'))

        folder_path = output_dir + '/' + str(game_id)
        os.makedirs(folder_path)
        
        n = len(receive_body['data'])
        for i in range(n):
            with open(folder_path + '/' + str(i) + '.json', 'w') as f:
                json.dump(receive_body['data'][i], f, indent=None)
        
        game_id += 1

data_server('./training_data')