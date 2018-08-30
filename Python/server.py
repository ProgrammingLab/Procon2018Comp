# coding: utf-8

from library import *
import sys
import os
import socket
import json
import collections


# TODO: DNNに入れる盤面の回転・反転など（学習時）


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

def to_states(json):
    res = []
    n = len(json['states'])
    for state_id in range(n):
        state_json = json['states'][state_id]
        res_turn = int(state_json['resTurn'])
        h = int(state_json['h'])
        w = int(state_json['w'])

        score = state_json['score']
        color = state_json['color']
        pos = state_json['pos']

        fld = [[Grid(0, 0) for j in range(w)] for i in range(h)]
        agent_pos = [[Pos(-1, -1) for j in range(2)] for i in range(2)]
        for i in range(h):
            for j in range(w):
                fld[i][j].score = int(score[i][j])
                fld[i][j].color = int(color[i][j])
        for i in range(2):
            for j in range(2):
                agent_pos[i][j].x = int(pos[i*2 + j]['x'])
                agent_pos[i][j].y = int(pos[i*2 + j]['y'])

        res.append(State(np.array(fld), agent_pos, res_turn))
    return res


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
            n = len(states)

            results = collections.OrderedDict()
            results['result'] = []
            for result_id in range(n):
                policy = [[0.0 for j in range(Move.max_int())] for i in range(2)]
                for i in range(2):
                    for j in range(Move.max_int()):
                        policy[i][j] = "{0:.16f}".format(policy_data[i][result_id][j])
                
                data = collections.OrderedDict()
                data['value'] = "{0:.16f}".format(value[result_id])
                data['policy'] = policy
                results['result'].append(data)
                
            sent = json.dumps(results, indent=None).encode('ascii')
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


dnn_server('./model/step=0.ckpt')
