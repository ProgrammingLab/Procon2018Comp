# coding: utf-8

from library import *
import sys
import os
import json
import numpy as np
import argparse
import collections
import shutil

def data_server(output_dir, all_game_count):
    if all_game_count == 0:
        return
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('', 54216))
    server_socket.listen(1)
    for game_id in range(all_game_count):
        (client_socket, address) = server_socket.accept()
        receive_body = json.loads(myreceive_with_sign(client_socket).decode('utf-8'))

        folder_path = output_dir + '/' + str(game_id)
        os.makedirs(folder_path)
        
        n = len(receive_body['data'])
        for i in range(n):
            with open(folder_path + '/' + str(i) + '.json', 'w') as f:
                json.dump(receive_body['data'][i], f, indent=None)
        sys.stdout.write('\rreceived train data %d/%d' % (game_id + 1, all_game_count))
    print()

train_data_dir = './garbage/'
print('waiting train data...')
if os.path.isdir(train_data_dir):
    shutil.rmtree(train_data_dir)
data_server(train_data_dir, 999999)
