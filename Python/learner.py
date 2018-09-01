# coding: utf-8

from library import *
import sys
import os
import json
import numpy as np

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

train_data_list = []
train_data_path = sys.argv[1]
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
    print('loaded ' + str(game_id) + '/' + str(game_count))

print(str(len(train_data_list)) + ' train data is here!')
dnn = Dnn('./model/step=0.ckpt')
for train_count in range(1000):
    print('trainStep: ' + str(train_count))
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
dnn.save('./model/test.ckpt')