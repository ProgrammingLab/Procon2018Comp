# coding: utf-8

from library import *

def to_train_data(json):
    state = to_state(json['state'])
    q = int(json['q'])
    z = int(json['z'])
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
        train_data.state.fld.transpose()
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
