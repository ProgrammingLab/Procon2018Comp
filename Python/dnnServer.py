# coding: utf-8

from library import *
import sys
import os
import socket
import json
import collections
import argparse

def to_states(json):
    res = []
    n = len(json['states'])
    for state_id in range(n):
        state_json = json['states'][state_id]
        res.append(to_state(state_json))
    return res

# 直に変更
def swap_player(state):
    for i in range(state.h()):
        for j in range(state.w()):
            c = state.fld[i][j].color
            state.fld[i][j].color = [0, 2, 1][c]
    p0 = state.agent_pos[0]
    p1 = state.agent_pos[1]
    state.agent_pos[0] = p1
    state.agent_pos[1] = p0

def dnn_server(model_path):
    dnn = Dnn(model_path)
    
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('localhost', 54215))
    server_socket.listen(1)
    print('started server')
    while True:
        (client_socket, address) = server_socket.accept()
        while True:
            receive_size = int(myreceive(client_socket, 10).decode('ascii'))
            # print(size)
            receive_body = json.loads(myreceive(client_socket, receive_size).decode('utf-8'))
            states = to_states(receive_body)
            n = len(states)
            swapped = [False for i in range(n)]
            for i in range(n):
                if np.random.randint(2) == 0:
                    swapped[i] = True
                    swap_player(states[i])

            # debug output
            # for state in states:
            #     print('resTurn: ' + str(state.res_turn))
            #     for i in range(state.h()):
            #         for j in range(state.w()):
            #             print(state.fld[i][j].score, end=' ')
            #         print()
            #     print()
            #     for i in range(state.h()):
            #         for j in range(state.w()):
            #             print(state.fld[i][j].color, end=' ')
            #         print()
            #     print()
            #     for i in range(2):
            #         for j in range(2):
            #             x = state.agent_pos[i][j].x
            #             y = state.agent_pos[i][j].y
            #             print('(x: ' + str(x) + ', y: ' + str(y) + ')')
            #     print()
            #     print()
            
            r = dnn.calc_batch(states)
            policy_data = r['policy_pair']
            value = r['value']

            results = collections.OrderedDict()
            results['result'] = []
            for result_id in range(n):
                policy = [[0.0 for j in range(Move.max_int())] for i in range(2)]
                for i in range(2):
                    i_ = i
                    if swapped[result_id]:
                        i_ = 1 - i
                    for j in range(Move.max_int()):
                        policy[i][j] = "{0:.16f}".format(policy_data[i_][result_id][j])
                
                v = value[result_id]
                if swapped[result_id]:
                    v *= -1.0
                data = collections.OrderedDict()
                data['value'] = "{0:.16f}".format(v)
                data['policy'] = policy
                results['result'].append(data)
                
            sent = json.dumps(results, indent=None).encode('utf-8')
            send_size_str = str(len(sent))
            if len(send_size_str) > 10:
                raise "too large"
            while len(send_size_str) < 10:
                send_size_str += ' '
            # print(sent)
            # print(send_size_str.encode('ascii'))
            mysend(client_socket, send_size_str.encode('ascii'))
            mysend(client_socket, sent)

            # print(result['value'])
            # print()
            # for p in result['policy_pair'][0]:
            #     print(p)
            # print()
            # for p in result['policy_pair'][1]:
            #     print(p)


# ベンチマーク的な
def benchmark(model_path):
    score = [
        [0, 0, 1, 0, 0],
        [0,-2, 1,-2, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0]
    ]
    color = [
        [1, 1, 0, 0, 1],
        [0, 0, 0, 0, 0],
        [0, 0, 0, 0, 0],
        [2, 0, 0, 0, 2]
    ]
    fld = [[Grid(score[i][j], color[i][j]) for j in range(5)] for i in range(4)]
    agent_pos = [[Pos(0, 0), Pos(4, 0)], [Pos(0, 3), Pos(4, 3)]]
    state = State(np.array(fld), agent_pos, 2)
    dnn = Dnn(model_path)
    mcts = MCTS(state, dnn)
    mcts.test()

parser = argparse.ArgumentParser()
parser.add_argument('model_path', \
    action='store', \
    nargs=None, \
    const=None, \
    default=None, \
    type=str, \
    choices=None, \
    help='Path to DNN model.', \
    metavar=None)
args = parser.parse_args()

dnn_server(args.model_path)