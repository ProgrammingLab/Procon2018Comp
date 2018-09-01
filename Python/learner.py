# coding: utf-8

from library import *
import sys
import os
import json
import numpy as np
import argparse
import collections

def to_train_data(json):
    state = to_state(json['state'])
    q = float(json['q'])
    z = float(json['z'])
    visit_counts = [[0.0 for j in range(Move.max_int())] for i in range(2)]
    for i in range(2):
        for j in range(Move.max_int()):
            visit_counts[i][j] = float(json['visitCount'][i][j])
    return TrainingData(state, visit_counts, q, z)

def swap_action(int_move):
    m = Move.from_int(int_move)
    m.action0, m.action1 = m.action1, m.action0
    return m.to_int()

def rot90(state):
    for i in range(2):
        for j in range(2):
            x = state.agent_pos[i][j].x
            y = state.agent_pos[i][j].y
            state.agent_pos[i][j].x = y
            state.agent_pos[i][j].y = state.w() - 1 - x
    state.fld = np.rot90(state.fld)

def shuffle_state(train_data):
    if np.random.randint(2) == 0: # 反転
        train_data.state.fld = train_data.state.fld.transpose()
        for i in range(2):
            for j in range(2):
                x = train_data.state.agent_pos[i][j].x
                y = train_data.state.agent_pos[i][j].y
                train_data.state.agent_pos[i][j].x = y
                train_data.state.agent_pos[i][j].y = x

    r = np.random.randint(4)
    for i in range(r): # 回転
        rot90(train_data.state)
    
    if np.random.randint(2) == 0: # プレイヤーの入れ替え
        for i in range(train_data.state.h()):
            for j in range(train_data.state.w()):
                c = train_data.state.fld[i][j].color
                train_data.state.fld[i][j].color = [0, 2, 1][c]
        p0 = train_data.state.agent_pos[0]
        p1 = train_data.state.agent_pos[1]
        train_data.state.agent_pos[0] = p1
        train_data.state.agent_pos[1] = p0
        train_data.q *= -1.0
        train_data.z *= -1.0
        v0 = train_data.visit_counts[0]
        v1 = train_data.visit_counts[1]
        train_data.visit_counts[0] = v1
        train_data.visit_counts[1] = v0

    for i in range(2):
        if np.random.randint(2) == 0: # エージェントの入れ替え
            p0 = train_data.state.agent_pos[i][0]
            p1 = train_data.state.agent_pos[i][1]
            train_data.state.agent_pos[i][0] = p1
            train_data.state.agent_pos[i][1] = p0
            v = [0.0 for m in range(Move.max_int())]
            for m in range(Move.max_int()):
                v[swap_action(m)] = train_data.visit_counts[i][m]
            train_data.visit_counts[i] = v

def learn(model_path, train_data_path, out_model_path):
    train_data_list = []
    game_count = 0
    while True:
        game_path = train_data_path + '/' + str(game_count)
        if not os.path.isdir(game_path):
            break
        game_count += 1
    for game_id in range(game_count):
        turn_id = 0
        game_path = train_data_path + '/' + str(game_id)
        while True:
            turn_path = game_path + '/' + str(turn_id) + '.json'
            if not os.path.exists(turn_path):
                break
            with open(turn_path) as f:
                data = to_train_data(json.load(f))
                data.name = turn_path
                train_data_list.append(data)
            turn_id += 1
        game_id += 1
        print('loaded %d/%d' % (game_id + 1, game_count))

    print(str(len(train_data_list)) + ' train data is here!')
    dnn = Dnn(model_path)
    for train_count in range(1000):
        sys.stdout.write('trainStep: %d' % train_count)
        batch = np.random.choice(train_data_list, 512, replace=False)
        states = []
        policies0 = []
        policies1 = []
        values = []
        for b in batch:
            shuffle_state(b)
            count_sum = [0, 0]
            for i in range(2):
                for j in range(Move.max_int()):
                    count_sum[i] += b.visit_counts[i][j]
            policy = [[0.0 for j in range(Move.max_int())] for i in range(2)]
            for i in range(2):
                for j in range(Move.max_int()):
                    policy[i][j] = b.visit_counts[i][j]/count_sum[i]
            states.append(b.state)
            policies0.append(policy[0])
            policies1.append(policy[1])
            values.append((b.q + b.z)/2)
        dnn.train(states, policies0, policies1, values, train_count)
    dnn.save(out_model_path)

def model_server(ckpt, model_dir, client_count, game_count):
    model_data = []
    header = collections.OrderedDict()
    header['modelFiles'] = []
    for file_name in os.listdir(model_dir):
        file_path = model_dir + '/' + file_name
        if os.path.isdir(file_path):
            continue
        header['modelFiles'].append(file_name)
        with open(file_path, 'rb') as f:
            model_data.append(f.read())
    header['checkpoint'] = str(ckpt)
    header['gameCount'] = str(game_count)
    byte_header = json.dumps(header, indent=None).encode('utf-8')
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('', 54217))
    server_socket.listen(1)
    for i in range(client_count):
        (client_socket, address) = server_socket.accept()
        mysend_with_sign(client_socket, byte_header)
        for d in model_data:
            mysend_with_sign(client_socket, d)
        print('sent model %d/%d' % (i + 1, client_count))

def data_server(output_dir, all_game_count):
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
        sys.stdout.write('received train data %d/%d' % (game_id + 1, all_game_count))


parser = argparse.ArgumentParser()
parser.add_argument('client_count', \
    action='store', \
    nargs=None, \
    const=None, \
    default=None, \
    type=int, \
    choices=None, \
    help='The number of clients', \
    metavar=None)
parser.add_argument('client_game_count', \
    action='store', \
    nargs=None, \
    const=None, \
    default=None, \
    type=int, \
    choices=None, \
    help='The number of games in a client', \
    metavar=None)
args = parser.parse_args()
client_count = args.clients
game_count = args.client_game_count

ckpt = 0
while True:
    s = './model/ckpt=%d' % ckpt
    if not os.path.isdir(s):
        break
ckpt -= 1
if ckpt == -1:
    print('error: initial model directory (./model/ckpt=0) is not found!!')
    exit()
print('./model/ckpt=%d is found' % ckpt)

while True:
    print('================')
    print('checkpoint %d' % ckpt)
    print('================')
    model_dir = './model/ckpt=' % ckpt
    train_data_dir = './trainData/ckpt=' + str(ckpt)
    model_path = model_dir + '/ckpt=' + str(ckpt)
    next_model_path = model_dir + '/ckpt=' + str(ckpt + 1)

    model_server(ckpt, model_dir, client_count, game_count)
    print('waiting train data...')
    data_server(train_data_dir, client_count*game_count)
    learn(model_path, train_data_dir, next_model_path)

    ckpt += 1
