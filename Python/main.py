# coding: utf-8

import numpy as np
import tensorflow as tf
import math
import copy
import random
import sys
import pickle
import os
import socket
import json
import collections

dx8 = [1, 0, -1, -1, -1, 0, 1, 1]
dy8 = [1, 1, 1, 0, -1, -1, -1, 0]
dx4 = [1, 0, -1, 0]
dy4 = [0, 1, 0, -1]

class Grid:
    def __init__(self, score, color):
        self.score = score
        self.color = color # 0:無色, 1:色0, 2:色1
    def __str__(self):
        return "0"

class Pos:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class State:
    def __init__(self, fld, agent_pos, res_turn):
        self.res_turn = res_turn
        self.fld = fld
        self.agent_pos = agent_pos
    def h(self):
        return self.fld.shape[0]
    def w(self):
        return self.fld.shape[1]
    # int, int, int
    @staticmethod
    def my_round(x):
        return math.floor(x + 0.5)
    @staticmethod
    def approximate_gause(n, start, end):
        return State.my_round(np.mean(np.random.randint(start, end, n)))
    @staticmethod
    def random_state():
        dh = State.approximate_gause(2, -MAX_H + 5, MAX_H - 5 + 1)
        dw = State.approximate_gause(2, -MAX_W + 5, MAX_W - 5 + 1)
        h = MAX_H - abs(dh)
        w = MAX_W - abs(dw)
        n = np.random.randint(1, 5)
        minus_p = np.random.rand()*0.3
        max_score = np.random.randint(1, 16 + 1)

        fld = np.array([[Grid(0, 0) for j in range(w)] for i in range(h)])
        for i in range(h):
            for j in range(w):
                if i >= (h + 1)//2: # 中央を境に線対称
                    fld[i][j] = copy.deepcopy(fld[h - 1 - i][j])
                    continue
                fld[i][j].score = abs(State.approximate_gause(n, -max_score, max_score + 1))
                if np.random.rand() < minus_p:
                    fld[i][j].score *= -1
                # if fld[i][j].score >= 0: print(" ", end="")
                # print(fld[i][j].score, end=" ")
            # print()
        # print()
        candidate = []
        for i in range(h//2):
            for j in range(w):
                candidate.append(Pos(j, i))
        random.shuffle(candidate)
        pos0 = candidate[0]
        pos1 = candidate[1]
        pos2 = Pos(pos0.x, h - 1 - pos0.y)
        pos3 = Pos(pos1.x, h - 1 - pos1.y)
        if np.random.rand() < 0.5:
            pos1, pos3 = pos3, pos1
        pos = [[pos0, pos1], [pos2, pos3]]
        for i in range(2):
            for j in range(2):
                fld[pos[i][j].y][pos[i][j].x].color = [1, 2][i]

        if np.random.randint(2) == 0: # ここまで列方向の対称しか考えていないので1/2の確率で転置
            h, w = w, h
            dh, dw = dw, dh
            fld = fld.transpose()
            for i in range(2):
                for j in range(2):
                    pos[i][j].x, pos[i][j].y = pos[i][j].y, pos[i][j].x
        
        # for i in range(h):
        #     for j in range(w):
        #         if fld[i][j].score >= 0:
        #             print(" ", end="")
        #         print(fld[i][j].score, end=" ")
        #     print()
        # print("------------------------------------------")
        # for i in range(h):
        #     for j in range(w):
        #         print(fld[i][j].color, end=" ")
        #     print()
        # print()
        # print()
        # print()
        
        # return State(fld, pos, 2)
        return State(fld, pos, np.random.randint(60, 120 + 1))
        
    @staticmethod
    def dfs(x, y, fld, res):
        h = fld.shape[0]
        w = fld.shape[1]
        res[y][x] = False
        for i in range(4):
            ny = y + dy4[i]
            nx = x + dx4[i]
            out_of_fld = ny < 0 or h <= ny or nx < 0 or w <= nx
            if out_of_fld or fld[ny][nx] or (not res[ny][nx]):
                continue
            State.dfs(nx, ny, fld, res)
    def evaluate_end(self):
        val = [0, 0]
        for i in range(2):
            h = self.fld.shape[0]
            w = self.fld.shape[1]
            fld = np.full((h + 2, w + 2), False)
            res = np.full((h + 2, w + 2), True)
            for y in range(h):
                for x in range(w):
                    if self.fld[y][x].color == [1, 2][i]:
                        fld[y + 1][x + 1] = True
            State.dfs(0, 0, fld, res)
            for y in range(h):
                for x in range(w):
                    if self.fld[y][x].color == [1, 2][i]:
                        val[i] += self.fld[y][x].score
                    if res[y + 1][x + 1] and self.fld[y][x].color == 0:
                        val[i] += abs(self.fld[y][x].score)
        if val[0] > val[1]:
            return 1
        if val[0] < val[1]:
            return -1
        return 0
    def valid_action(self, player_id, agent_id, action):
        if action == Action.waiting():
            return True
        ty = self.agent_pos[player_id][agent_id].y + dy8[action.dir8]
        tx = self.agent_pos[player_id][agent_id].x + dx8[action.dir8]
        h = self.fld.shape[0]
        w = self.fld.shape[1]
        if ty < 0 or h <= ty or tx < 0 or w <= tx:
            return False
        if action.is_remove:
            return self.fld[ty][tx].color != 0
        return self.fld[ty][tx].color != [2,1][player_id]
    def valid_move(self, player_id, move):
        b0 = self.valid_action(player_id, 0, move.action0)
        b1 = self.valid_action(player_id, 1, move.action1)
        return b0 and b1
    # return: {ゲーム続行: None, ゲーム終了: 評価値(-1 or 0 or 1)}
    def next(self, move0, move1):
        actions = [[move0.action0, move0.action1], [move1.action0, move1.action1]]
        def target_pos(i, j):
            y = self.agent_pos[i][j].y
            x = self.agent_pos[i][j].x
            if actions[i][j] == Action.waiting():
                return (x, y)
            ty = y + dy8[actions[i][j].dir8]
            tx = x + dx8[actions[i][j].dir8]
            return (tx, ty)
        def ok(i, j):
            if actions[i][j] == Action.waiting():
                return True
            (tx, ty) = target_pos(i, j)
            is_remove = actions[i][j].is_remove
            for s in range(2):
                for t in range(2):
                    if i == s and j == t:
                        continue
                    (tx_, ty_) = target_pos(s, t)
                    is_remove_ = actions[s][t].is_remove
                    if tx == tx_ and ty == ty_ and is_remove == is_remove_:
                        return False
            return True
        for i in range(2):
            for j in range(2):
                if actions[i][j] == Action.waiting():
                    continue
                (tx, ty) = target_pos(i, j)
                if actions[i][j].is_remove:
                    self.fld[ty][tx].color = 0
                else:
                    if ok(i, j):
                        self.agent_pos[i][j] = Pos(tx, ty)
        for i in range(2):
            for j in range(2):
                y = self.agent_pos[i][j].y
                x = self.agent_pos[i][j].x
                # print((y, x), (self.fld.shape))
                self.fld[y][x].color = [1, 2][i]
        self.res_turn -= 1
        if self.res_turn == 0:
            return self.evaluate_end()
        return None
class Action:
    def __init__(self, is_remove, dir8):
        self.is_remove = is_remove
        self.dir8 = dir8
    @staticmethod
    def waiting():
        return Action(False, -1)
    @staticmethod
    def from_int(i):
        if i == 0:
            return Action.waiting()
        if i >= 9:
            return Action(True, i - 9)
        return Action(False, i - 1)
    def to_int(self):
        return (8 if self.is_remove else 0) + self.dir8 + 1
    def __eq__(self, other):
        return self.is_remove == other.is_remove and self.dir8 == other.dir8
    def __ne__(self, other):
        return self.is_remove != other.is_remove or self.dir8 != other.dir8

class Move:
    def __init__(self, action0, action1):
        self.action0 = action0
        self.action1 = action1
    def to_int(self):
        return self.action0.to_int()*17 + self.action1.to_int()
    @staticmethod
    def from_int(i):
        return Move(Action.from_int(i//17), Action.from_int(i%17))
    @staticmethod
    def max_int():
        return 17*17

class Dnn:
    def __init__(self, model_path):
        # TODO?: 過学習抑制
        self.is_training = tf.placeholder(tf.bool, shape=[])
        self.x = tf.placeholder(tf.float32, shape=[None, MAX_H, MAX_W, 8])
        self.policies0_ = tf.placeholder(tf.float32, shape=[None, Move.max_int()])
        self.policies1_ = tf.placeholder(tf.float32, shape=[None, Move.max_int()])
        self.values_ = tf.placeholder(tf.float32)

        x_ = Dnn.input_to_res_block(self.x, self.is_training)
        r0 = Dnn.res_block(x_, self.is_training)
        r1 = Dnn.res_block(r0, self.is_training)
        r2 = Dnn.res_block(r1, self.is_training)
        r3 = Dnn.res_block(r2, self.is_training)
        r4 = Dnn.res_block(r3, self.is_training)
        r5 = Dnn.res_block(r4, self.is_training)
        r6 = Dnn.res_block(r5, self.is_training)
        r7 = Dnn.res_block(r6, self.is_training)
        r8 = Dnn.res_block(r7, self.is_training)
        r9 = Dnn.res_block(r8, self.is_training)
        r10 = Dnn.res_block(r9, self.is_training)
        r11 = Dnn.res_block(r10, self.is_training)
        r12 = Dnn.res_block(r11, self.is_training)
        self.values = Dnn.value_out(r12, self.is_training)
        self.value_loss = (self.values_ - self.values)**2
        (logits0, logits1) = self.policy_out(r12, self.is_training)
        self.policy0 = tf.nn.softmax(logits0)
        self.policy1 = tf.nn.softmax(logits1)
        self.policy0_loss = tf.nn.softmax_cross_entropy_with_logits_v2(labels=self.policies0_, logits=logits0)
        self.policy1_loss = tf.nn.softmax_cross_entropy_with_logits_v2(labels=self.policies1_, logits=logits1)
        self.loss = self.value_loss + self.policy0_loss + self.policy1_loss
        self.optimizer = tf.train.GradientDescentOptimizer(1e-4)

        update_ops = tf.get_collection(tf.GraphKeys.UPDATE_OPS)
        with tf.control_dependencies(update_ops):
            self.train_op = self.optimizer.minimize(self.loss)

        config = tf.ConfigProto(
            device_count={"GPU":1},
            log_device_placement=False,
            gpu_options=tf.GPUOptions(
                #per_process_gpu_memory_fraction=0.1
            )
        )

        self.sess = tf.Session(config=config)
        # with tf.name_scope('summary'):
        #     writer = tf.summary.FileWriter('./logs', self.sess.graph)
        if model_path:
            tf.train.Saver().restore(self.sess, model_path)
        else:
            self.sess.run(tf.global_variables_initializer())
    def save(self, path):
        saver = tf.train.Saver()
        saver.save(self.sess, path)
    @staticmethod
    def he_initializer(n):
        return tf.initializers.truncated_normal(stddev=math.sqrt(2.0/n))
    @staticmethod
    def xavier_initializer(n):
        return tf.initializers.truncated_normal(stddev=math.sqrt(1/n))
    @staticmethod
    def input_to_res_block(input_layer, is_training):
        shape = input_layer.get_shape().as_list()
        n = shape[1]*shape[2]*shape[3]
        conv = tf.layers.conv2d(
            inputs=input_layer,
            filters=64,
            kernel_size=[3,3],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n))
        bn = tf.layers.batch_normalization(
            conv,
            training=is_training)
        return tf.nn.relu(bn)
    @staticmethod
    def res_block(input_layer, is_training):
        bn0 = tf.layers.batch_normalization(
            input_layer,
            training=is_training)
        shape0 = bn0.get_shape().as_list()
        n0 = shape0[1]*shape0[2]*shape0[3]
        conv0 = tf.layers.conv2d(
            inputs=bn0,
            filters=64,
            kernel_size=[3,3],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n0))
        bn1 = tf.layers.batch_normalization(
            conv0,
            training=is_training)
        relu = tf.nn.relu(bn1)
        shape1 = relu.get_shape().as_list()
        n1 = shape1[1]*shape1[2]*shape1[3]
        conv1 = tf.layers.conv2d(
            inputs=relu,
            filters=64,
            kernel_size=[3,3],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n1))
        return tf.add(conv1, input_layer)
    # tensorflowの関数の関係上、softmaxまでせずに返す
    @staticmethod
    def policy_out(input_layer, is_training):
        # batch_normalizationの順番どうすべき？
        # → pre-activation版のres-blockの性質がよくわからないが,
        #   原論文でres-block → avr-pooling → fully-connectedしてる辺り見ると
        #   多分いきなり畳み込んで大丈夫
        shape0 = input_layer.get_shape().as_list()
        n0 = shape0[1]*shape0[2]*shape0[3]
        conv = tf.layers.conv2d(
            inputs=input_layer,
            filters=2,
            kernel_size=[1,1],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n0))
        bn = tf.layers.batch_normalization(
            conv,
            training=is_training)
        relu = tf.nn.relu(bn)
        n1 = MAX_H*MAX_W*2
        relu_flat = tf.reshape(relu, [-1, n1])
        # TODO?: he_initializeの方が良いかも
        logits0 = tf.layers.dense(
            relu_flat,
            units=Move.max_int(),
            kernel_initializer=Dnn.xavier_initializer(n1))
        logits1 = tf.layers.dense(
            relu_flat,
            units=Move.max_int(),
            kernel_initializer=Dnn.xavier_initializer(n1))
        return (logits0, logits1)
    @staticmethod
    def value_out(input_layer, is_training):
        shape0 = input_layer.get_shape().as_list()
        n0 = shape0[1]*shape0[2]*shape0[3]
        conv = tf.layers.conv2d(
            inputs=input_layer,
            filters=1,
            kernel_size=[1,1],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n0))
        bn = tf.layers.batch_normalization(
            conv,
            training=is_training)
        relu = tf.nn.relu(bn)
        n1 = MAX_H*MAX_W*1
        relu_flat = tf.reshape(relu, [-1, n1])
        dense0 = tf.layers.dense(
            relu_flat,
            units=64,
            kernel_initializer=Dnn.he_initializer(n1))
        n2 = 64
        dense1 = tf.layers.dense(
            dense0,
            units=1,
            kernel_initializer=Dnn.xavier_initializer(n2))
        return tf.nn.tanh(dense1[0])
    @staticmethod
    def adjust_to_dnn(states):
        cases = len(states)
        x = np.zeros([cases, MAX_H, MAX_W, 8])
        for case_id in range(cases):
            state = states[case_id]
            
            o_r = np.random.randint(MAX_H - state.h() + 1)
            o_c = np.random.randint(MAX_W - state.w() + 1)
            for i in range(MAX_H):
                for j in range(MAX_W):
                    if i < o_r or o_r + state.h() <= i or j < o_c or o_c + state.w() <= j:
                        x[case_id][i][j][1] = 1.0
                        continue
                    x[case_id][i][j][0] = state.fld[i - o_r][j - o_c].score
                    c = state.fld[i - o_r][j - o_c].color
                    if c == 1:
                        x[case_id][i][j][2] = 1.0
                    if c == 2:
                        x[case_id][i][j][3] = 1.0
            p00 = state.agent_pos[0][0]
            p01 = state.agent_pos[0][1]
            p10 = state.agent_pos[1][0]
            p11 = state.agent_pos[1][1]
            x[case_id][o_r + p00.y][o_c + p00.x][4] = 1.0
            x[case_id][o_r + p01.y][o_c + p01.x][5] = 1.0
            x[case_id][o_r + p10.y][o_c + p10.x][6] = 1.0
            x[case_id][o_r + p11.y][o_c + p11.x][7] = 1.0
        return x
    # return: {'policy_pair': (Move, Move), 'value': float}
    # valueはプレイヤー0にとって正で、[-1, 1]
    def calc(self, state):
        feed_dict={self.x:Dnn.adjust_to_dnn([state]), self.is_training:False}
        result = self.sess.run([self.values, self.policy0, self.policy1], feed_dict=feed_dict)
        return {'policy_pair':(result[1][0], result[2][0]), 'value':result[0][0]}

    # return: {'policy_pair': ([Move], [Move]), 'value': [float]}
    def calc_batch(self, states):
        feed_dict={self.x:Dnn.adjust_to_dnn(states), self.is_training:False}
        result = self.sess.run([self.values, self.policy0, self.policy1], feed_dict=feed_dict)
        return {'policy_pair':(result[1], result[2]), 'value':result[0]}

class Node:
    # 実体化するのは訪れる時だけ（展開時は実体化まではしない）
    def __init__(self, policy_pair):
        self.w = [[0.0]*Move.max_int(), [0.0]*Move.max_int()]
        self.count = [[0]*Move.max_int(), [0]*Move.max_int()]
        self.count_sum = 0
        self.policy = policy_pair
        self.next = {}
    # player_id: 0 or 1
    def decide_move(self, player_id, state):
        max = -1e10
        max_a = -1
        for a in range(Move.max_int()):
            if not state.valid_move(player_id, Move.from_int(a)):
                continue
            c = 1.0
            w = self.w[player_id][a]
            n = self.count[player_id][a]
            q = w/n if n > 0 else 0
            p = self.policy[player_id][a]
            u = c*p*math.sqrt(self.count_sum)/(1 + n)
            if max_a == -1 or max < q + u:
                max = q + u
                max_a = a
        return Move.from_int(max_a)
    # return: (Move, Move)
    def decide_moves(self, state):
        return (self.decide_move(0, state), self.decide_move(1, state))
    def backup(self, move_int_pair, v):
        a0 = move_int_pair[0]
        a1 = move_int_pair[1]
        self.count_sum += 1
        self.count[0][a0] += 1
        self.count[1][a1] += 1
        self.w[0][a0] += v
        self.w[1][a1] += -v

class MCTS:
    def __init__(self, state, dnn):
        self.dnn = dnn
        self.first_state = state
        self.root = Node(dnn.calc(state)['policy_pair'])
    def go_down(self, node, state):
        move_pair = node.decide_moves(state)
        move_int_pair = (move_pair[0].to_int(), move_pair[1].to_int())
        v = state.next(move_pair[0], move_pair[1])
        if v is None:
            is_leaf = not (move_int_pair in node.next)
            if is_leaf: # 子ノードが葉だったら実体化&評価
                res = self.dnn.calc(state)
                node.next[move_int_pair] = Node(res['policy_pair'])
                v = res['value']
            else:
                v = self.go_down(node.next[move_int_pair], state)
        node.backup(move_int_pair, v)
        return v
    # 探索するだけで盤面は変えない
    # Q値を返す
    def search(self, loop):
        q = 0
        for i in range(loop):
            q += self.go_down(self.root, copy.deepcopy(self.first_state))
        return q/loop
    # 自己対局を1ターン進める. 根ノードも取り替える.
    # temperatureが1に近づくほど, 訪問回数に比例した確率で手を選ぶ
    # temperatureが0に近づくほど, 訪問回数の多い手をひいきする
    def self_next(self, temperature):
        int_moves = []
        if (temperature < 1e-3): # 単に訪問回数最大の手を選ぶ
            for i in range(2):
                max = 0
                max_i = -1
                for j in range(Move.max_int()):
                    if max < self.root.count[i][j]:
                        max = self.root.count[i][j]
                        max_i = j
                int_moves.append(max_i)
        else:
            for i in range(2):
                sum = 0
                for i in range(Move.max_int()):
                    sum += self.root.count[i][j]**(1/temperature)
                p = [0]*Move.max_int()
                for j in range(Move.max_int()):
                    p[j] = (self.root.count[i][j]**(1/temperature))/sum
                int_moves.append(np.random.choice(Move.max_int(), p=p))
        int_move_pair = (int_moves[0], int_moves[1])
        move_pair = (Move.from_int(int_move_pair[0]), Move.from_int(int_move_pair[1]))
        v = self.first_state.next(move_pair[0], move_pair[1])
        if not (v is None): # ゲーム終了
            self.root = None
        elif int_move_pair in self.root.next:
            self.root = self.root.next[int_move_pair]
        else: # 移動先ノードがまだ実体化されていなかった(DUCTなので起こりうる)
            res = self.dnn.calc(self.first_state)
            self.root = Node(res['policy_pair'])
    def test(self):
        for i in range(1000):
            if i%100 == 0: print(i)
            self.go_down(self.root, copy.deepcopy(self.first_state))
        def to_str(a):
            if a == Action.waiting():
                return " x"
            res = "m"
            if a.is_remove:
                res = "r"
            return res + str(a.dir8)
        for i in range(2):
            print("player" + str(i) + ":")
            for j in range(Move.max_int()):
                m = Move.from_int(j)
                n = self.root.count[i][j]
                w = self.root.w[i][j]
                q = w/n if n > 0 else 0
                print("  " + to_str(m.action0) + ", " + to_str(m.action1) + " : " + str(n) + ", " + str(q))

MAX_H = 12
MAX_W = 12

class TrainingData:
    def __init__(self, state, visit_counts, q, z):
        self.state = state
        self.visit_counts = visit_counts
        self.q = q
        self.z = z

def self_play(dnn, output_dir):
    game_count = 0
    while True:
        state_list = []
        visit_counts_list = []
        q_list = []
        mcts = MCTS(State.random_state(), dnn)
        while mcts.first_state.res_turn > 0:
            print(str(game_count) + '-' + str(mcts.first_state.res_turn))
            try:
                with open('stop.command', 'r'):
                    print('command: stop!')
                    return
            except FileNotFoundError:
                pass
            state_list.append(copy.deepcopy(mcts.first_state))
            q = mcts.search(1000)
            visit_counts_list.append(copy.deepcopy(mcts.root.count))
            q_list.append(q)
            mcts.self_next(0)
        z = mcts.first_state.evaluate_end()
        for i in range(len(state_list)):
            data = TrainingData(state_list[i], visit_counts_list[i], q_list[i], z)
            path = output_dir
            path += 'game=' + str(game_count) + '_turn=' + str(i)
            path += '.TrainingData.pickle'
            with open(path, 'wb') as f:
                pickle.dump(data, f)
        game_count += 1



# =========================以下グローバル関数と実行部=======================
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
    print('totalsent: ' + str(totalsent))

def toStates(json):
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



# 自己対局
def SelfPlay():
    output_dir = sys.argv[1]
    # TODO: DNNの読み込み
    dnn = Dnn('./model/step=0.ckpt')
    try:
        os.makedirs(output_dir)
    except FileExistsError:
        pass
    # import cProfile
    # cProfile.run('self_play(dnn, output_dir)', 'profile.stats')
    self_play(dnn, output_dir)


def DnnServer(model_path):
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
            states = toStates(receive_body)

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

# pickleテスト
def PickelTest():
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
    import pickle
    with open('test.pickle', mode='wb') as f:
        pickle.dump(state, f)
    with open('test.pickle', mode='rb') as f:
        state_ = pickle.load(f)
    for i in range(state_.h()):
        for j in range(state_.w()):
            print(state_.fld[i][j].score, end=" ")
        print()

# ベンチマーク的な
def Benchmark(model_path):
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


DnnServer('./model/step=0.ckpt')
