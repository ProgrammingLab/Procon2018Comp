# coding: utf-8

import numpy as np
import tensorflow as tf
import math
import copy
import random
import json
import socket
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
    def equals(self, p):
        return self.x == p.x and self.y == p.y

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
        
        # return State(fld, pos, 2)
        return State(fld, pos, np.random.randint(60, 120 + 1))
    
    def print(self):
        print('=================================================')
        for i in range(self.h()):
            for j in range(self.w()):
                print('{:>3}'.format(self.fld[i][j].score), end='')
            print()
        print()
        for i in range(self.h()):
            for j in range(self.w()):
                print('  ', end='')
                p = Pos(j, i)
                if p.equals(self.agent_pos[0][0]):
                    print('A', end='')
                elif p.equals(self.agent_pos[0][1]):
                    print('a', end='')
                elif p.equals(self.agent_pos[1][0]):
                    print('B', end='')
                elif p.equals(self.agent_pos[1][1]):
                    print('b', end='')
                elif self.fld[i][j].color == 0:
                    print('.', end='')
                else:
                    print(self.fld[i][j].color - 1, end='')
            print()
        print('=================================================')
        
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
    def to_json(self):
        js = collections.OrderedDict()
        js['resTurn'] = str(self.res_turn)
        js['h'] = str(self.h())
        js['w'] = str(self.w())
        score = [[0 for j in range(self.w())] for j in range(self.h())]
        color = [[0 for j in range(self.w())] for j in range(self.h())]
        for i in range(self.h()):
            for j in range(self.w()):
                score[i][j] = str(self.fld[i][j].score)
                color[i][j] = str(self.fld[i][j].color)
        js['score'] = score
        js['color'] = color
        pos = []
        for i in range(2):
            for j in range(2):
                p = collections.OrderedDict()
                p['x'] = str(self.agent_pos[i][j].x)
                p['y'] = str(self.agent_pos[i][j].y)
                pos.append(p)
        js['pos'] = pos
        return js

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
    def __init__(self, model_path, log_dir=None):
        self.is_training = tf.placeholder(tf.bool, shape=[])
        self.input_dropout_rate = tf.placeholder(tf.float32, shape=[])
        self.hidden_dropout_rate = tf.placeholder(tf.float32, shape=[])
        self.x = tf.placeholder(tf.float32, shape=[None, MAX_H, MAX_W, 9])
        self.policies0_ = tf.placeholder(tf.float32, shape=[None, MAX_H*MAX_W*2])
        self.policies1_ = tf.placeholder(tf.float32, shape=[None, MAX_H*MAX_W*2])
        self.policies2_ = tf.placeholder(tf.float32, shape=[None, MAX_H*MAX_W*2])
        self.policies3_ = tf.placeholder(tf.float32, shape=[None, MAX_H*MAX_W*2])
        self.values_ = tf.placeholder(tf.float32)

        x_ = self.first_res_block(self.x)
        r0 = self.res_block(x_)
        r1 = self.res_block(r0)
        r2 = self.res_block(r1)
        r3 = self.res_block(r2)
        r4 = self.res_block(r3)
        r5 = self.res_block(r4)
        r6 = self.res_block(r5)
        r7 = self.res_block(r6)
        r8 = self.res_block(r7)
        r9 = self.res_block(r8)
        r10 = self.res_block(r9)
        r11 = self.res_block(r10)
        r12 = self.res_block(r11)
        self.values = self.value_out(r12)
        self.value_loss = tf.reduce_mean((self.values_ - self.values)**2)
        self.logits0 = self.policy_out(r12)
        self.logits1 = self.policy_out(r12)
        self.logits2 = self.policy_out(r12)
        self.logits3 = self.policy_out(r12)
        self.policies0 = tf.nn.softmax(self.logits0)
        self.policies1 = tf.nn.softmax(self.logits1)
        self.policies2 = tf.nn.softmax(self.logits2)
        self.policies3 = tf.nn.softmax(self.logits3)
        self.policy0_loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(labels=self.policies0_, logits=self.logits0))
        self.policy1_loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(labels=self.policies1_, logits=self.logits1))
        self.policy2_loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(labels=self.policies2_, logits=self.logits2))
        self.policy3_loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits_v2(labels=self.policies3_, logits=self.logits3))
        self.policy_loss = (self.policy0_loss + self.policy1_loss + self.policy2_loss + self.policy3_loss)/4.0
        # self.regularization_loss = 0.0001*tf.losses.get_regularization_loss()
        self.loss = self.value_loss + self.policy_loss #+ self.regularization_loss
        self.optimizer = tf.train.AdamOptimizer(learning_rate=1e-3)

        update_ops = tf.get_collection(tf.GraphKeys.UPDATE_OPS)
        with tf.control_dependencies(update_ops):
            self.train_op = self.optimizer.minimize(self.loss)
        

        config = tf.ConfigProto(
            log_device_placement=False,
            gpu_options=tf.GPUOptions(
                #per_process_gpu_memory_fraction=0.1
            )
        )

        self.sess = tf.Session(config=config)
        if log_dir:
            with tf.name_scope('summary'):
                tf.summary.scalar('loss', self.loss)
                tf.summary.scalar('valueLoss', self.value_loss)
                tf.summary.scalar('policyLoss', self.policy_loss)
                # tf.summary.scalar('regularizationLoss', self.regularization_loss)
                self.summary_op = tf.summary.merge_all()
                self.summary_writer = tf.summary.FileWriter(log_dir, self.sess.graph)
        if model_path:
            tf.train.Saver().restore(self.sess, model_path)
        else:
            self.sess.run(tf.global_variables_initializer())
    
    def save(self, path):
        saver = tf.train.Saver()
        saver.save(self.sess, path)
    
    def train(self, states, policies0, policies1, values, steps):
        x, positions = Dnn.adjust_to_dnn(states)
        p0_, p1_, p2_, p3_ = Dnn.adjust_policy(policies0, policies1, positions)
        feed_dict = {
            self.x: x,
            self.policies0_: p0_,
            self.policies1_: p1_,
            self.policies2_: p2_,
            self.policies3_: p3_,
            self.values_: values,
            self.is_training: True,
            self.input_dropout_rate: 0.1,
            self.hidden_dropout_rate: 0.3
        }
        _, w_summary = self.sess.run([self.train_op, self.summary_op], feed_dict=feed_dict)
        self.summary_writer.add_summary(w_summary, steps)

    @staticmethod
    def he_initializer(n):
        return tf.initializers.truncated_normal(stddev=math.sqrt(2.0/n))
    @staticmethod
    def xavier_initializer(n):
        return tf.initializers.truncated_normal(stddev=math.sqrt(1/n))

    def first_res_block(self, input):
        bn0 = tf.layers.batch_normalization(
            input,
            training=self.is_training)
        relu0 = tf.nn.relu(bn0)
        shape0 = relu0.get_shape().as_list()
        n0 = shape0[1]*shape0[2]*shape0[3]
        conv0 = tf.layers.conv2d(
            inputs=relu0,
            filters=64,
            kernel_size=[3,3],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n0),
            kernel_regularizer= tf.contrib.layers.l2_regularizer(scale=1.0))
        bn1 = tf.layers.batch_normalization(
            conv0,
            training=self.is_training)
        relu1 = tf.nn.relu(bn1)
        dr = tf.layers.dropout(relu1, rate=self.input_dropout_rate)
        shape1 = dr.get_shape().as_list()
        n1 = shape1[1]*shape1[2]*shape1[3]
        conv1 = tf.layers.conv2d(
            inputs=dr,
            filters=64,
            kernel_size=[3,3],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n1),
            kernel_regularizer= tf.contrib.layers.l2_regularizer(scale=1.0))

        ishape = input.get_shape().as_list()
        ni = ishape[1]*ishape[2]*ishape[3]
        input_flat = tf.reshape(input, [-1, ni])
        ni_ = ishape[1]*ishape[2]*64
        input_dense = tf.layers.dense(
            input_flat,
            units=ni_,
            kernel_initializer=Dnn.xavier_initializer(ni_),
            kernel_regularizer=tf.contrib.layers.l2_regularizer(scale=1.0))
        input_ = tf.reshape(input_dense, [-1, ishape[1], ishape[2], 64])
        input_dr = tf.layers.dropout(input_, rate=self.input_dropout_rate)
        return tf.add(conv1, input_dr)

    def res_block(self, input):
        bn0 = tf.layers.batch_normalization(
            input,
            training=self.is_training)
        relu0 = tf.nn.relu(bn0)
        shape0 = relu0.get_shape().as_list()
        n0 = shape0[1]*shape0[2]*shape0[3]
        conv0 = tf.layers.conv2d(
            inputs=relu0,
            filters=64,
            kernel_size=[3,3],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n0),
            kernel_regularizer= tf.contrib.layers.l2_regularizer(scale=1.0))
        bn1 = tf.layers.batch_normalization(
            conv0,
            training=self.is_training)
        relu1 = tf.nn.relu(bn1)
        dr = tf.layers.dropout(relu1, rate=self.hidden_dropout_rate)
        shape1 = dr.get_shape().as_list()
        n1 = shape1[1]*shape1[2]*shape1[3]
        conv1 = tf.layers.conv2d(
            inputs=dr,
            filters=64,
            kernel_size=[3,3],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n1),
            kernel_regularizer= tf.contrib.layers.l2_regularizer(scale=1.0))
        return tf.add(conv1, input)

    # tensorflowの関数の関係上、softmaxまでせずに返す
    def policy_out(self, input):
        # batch_normalizationの順番どうすべき？
        # → pre-activation版のres-blockの性質がよくわからないが,
        #   原論文でres-block → avr-pooling → fully-connectedしてる辺り見ると
        #   多分いきなり畳み込んで大丈夫
        # → res-blockの後だからちゃんとbnとreluをしておかなければならないのではと思いました
        #   fully-connectedの前にはいらなさそう
        bn0 = tf.layers.batch_normalization(
            input,
            training=self.is_training)
        relu0 = tf.nn.relu(bn0)
        
        shape0 = relu0.get_shape().as_list()
        n0 = shape0[1]*shape0[2]*shape0[3]
        logits = tf.layers.conv2d(
            inputs=relu0,
            filters=2,
            kernel_size=[1,1],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n0),
            kernel_regularizer= tf.contrib.layers.l2_regularizer(scale=1.0))
        return tf.reshape(logits, [-1, MAX_H*MAX_W*2])

    def value_out(self, input):
        bn0 = tf.layers.batch_normalization(
            input,
            training=self.is_training)
        relu0 = tf.nn.relu(bn0)
        shape0 = relu0.get_shape().as_list()
        n0 = shape0[1]*shape0[2]*shape0[3]
        conv = tf.layers.conv2d(
            inputs=relu0,
            filters=1,
            kernel_size=[1,1],
            padding="same",
            kernel_initializer=Dnn.he_initializer(n0),
            kernel_regularizer= tf.contrib.layers.l2_regularizer(scale=1.0))
        bn1 = tf.layers.batch_normalization(
            conv,
            training=self.is_training)
        relu1 = tf.nn.relu(bn1)
        dr0 = tf.layers.dropout(relu1, rate=self.hidden_dropout_rate)
        n1 = MAX_H*MAX_W*1
        dr0_flat = tf.reshape(dr0, [-1, n1])
        dense0 = tf.layers.dense(
            dr0_flat,
            units=64,
            kernel_initializer=Dnn.he_initializer(n1),
            kernel_regularizer= tf.contrib.layers.l2_regularizer(scale=1.0))
        relu2 = tf.nn.relu(dense0)
        dr1 = tf.layers.dropout(relu2, rate=self.hidden_dropout_rate)
        n2 = 64
        dense1 = tf.layers.dense(
            dr1,
            units=1,
            kernel_initializer=Dnn.xavier_initializer(n2),
            kernel_regularizer= tf.contrib.layers.l2_regularizer(scale=1.0))
        return tf.nn.tanh(tf.reshape(dense1, [-1]))
    @staticmethod
    def adjust_policy(policies0, policies1, positions):
        cases = len(policies0)
        p0 = np.zeros([cases, MAX_H, MAX_W, 2], dtype=float32)
        p1 = np.zeros([cases, MAX_H, MAX_W, 2], dtype=float32)
        p2 = np.zeros([cases, MAX_H, MAX_W, 2], dtype=float32)
        p3 = np.zeros([cases, MAX_H, MAX_W, 2], dtype=float32)
        p0_ = np.reshape(p0, [cases, MAX_H*MAX_W*2])
        p1_ = np.reshape(p1, [cases, MAX_H*MAX_W*2])
        p2_ = np.reshape(p2, [cases, MAX_H*MAX_W*2])
        p3_ = np.reshape(p3, [cases, MAX_H*MAX_W*2])
        for case_id in range(cases):
            o_r = o_r_list[case_id]
            o_c = o_c_lsit[case_id]

            def add(agent_id, action, v):
                trg = (p0, p1, p2, p3)[agent_id]
                pos = positions[case_id][agent_id]
                channel = 1 if action.is_remove else 0
                pos_ = Pos(pos.x, pos.y)
                if action != Action.waiting():
                    pos_.x += dx8[action.dir8]
                    pos_.y += dy8[action.dir8]
                trg[case_id][pos_.y][pos_.x][channel] += v

            for i in range(Move.max_int()):
                m = Move.from_int(i)
                add(0, m.action0, policies0[case_id][i])
                add(1, m.action1, policies0[case_id][i])
                add(2, m.action0, policies1[case_id][i])
                add(3, m.action1, policies1[case_id][i])
        return p0_, p1_, p2_, p3_
    @staticmethod
    def unadjust_policy(p0_, p1_, p2_, p3_, positions):
        cases = len(p0_)
        p0 = np.reshape(p0_, [cases, MAX_H, MAX_W, 2])
        p1 = np.reshape(p1_, [cases, MAX_H, MAX_W, 2])
        p2 = np.reshape(p2_, [cases, MAX_H, MAX_W, 2])
        p3 = np.reshape(p3_, [cases, MAX_H, MAX_W, 2])
        ret0 = np.empty([cases, Move.max_int()])
        ret1 = np.empty([cases, Move.max_int()])
        for case_id in range(cases):
            def get(agent_id, action):
                pos = positions[case_id][agent_id]
                p = (p0, p1, p2, p3)[agent_id]
                pos_ = Pos(pos.x, pos.y)
                channel = 1 if action.is_remove else 0
                if action != Action.waiting():
                    pos_.x += dx8[action.dir8]
                    pos_.y += dy8[action.dir8]
                return p[case_id][pos_.y][pos_.x][channel]
            for i in range(Move.max_int()):
                m = Move.from_int(i)
                ret0[case_id][i] = get(0, m.action0)*get(1, m.action1)
                ret1[case_id][i] = get(2, m.action0)*get(3, m.action1)
        return ret0, ret1
    @staticmethod
    def adjust_to_dnn(states):
        cases = len(states)
        x = np.zeros([cases, MAX_H, MAX_W, 9], dtype=float32)
        positions = [[Pos(0, 0) for j in range(4)] for i in range(cases)]
        
        for case_id in range(cases):
            state = states[case_id]
            
            o_r = np.random.randint(MAX_H - state.h() + 1)
            o_c = np.random.randint(MAX_W - state.w() + 1)
            for i in range(MAX_H):
                for j in range(MAX_W):
                    x[case_id][i][j][8] = float(state.res_turn)
                    if i < o_r or o_r + state.h() <= i or j < o_c or o_c + state.w() <= j:
                        x[case_id][i][j][1] = 1.0
                        continue
                    x[case_id][i][j][0] = float(state.fld[i - o_r][j - o_c].score)
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
            positions[case_id][0] = Pos(o_r + p00.y, o_c + p00.x)
            positions[case_id][1] = Pos(o_r + p01.y, o_c + p01.x)
            positions[case_id][2] = Pos(o_r + p10.y, o_c + p10.x)
            positions[case_id][3] = Pos(o_r + p11.y, o_c + p11.x)

        return x, positions
    # return: {'policy_pair': (Move, Move), 'value': float}
    # valueはプレイヤー0にとって正で、[-1, 1]
    def calc(self, state):
        x, positions = Dnn.adjust_to_dnn([state])
        feed_dict = {
            self.x: x,
            self.is_training: False,
            self.input_dropout_rate: 0,
            self.hidden_dropout_rate: 0
        }
        result = self.sess.run([self.values, self.policies0, self.policies1, self.policies2, self.policies3], feed_dict=feed_dict)
        pp = Dnn.unadjust_policy(result[1], result[2], result[3], result[4], positions)
        return {'policy_pair':(pp[0][0], pp[1][0]), 'value':result[0][0]}

    # return: {'policy_pair': ([Move], [Move]), 'value': [float]}
    def calc_batch(self, states):
        x, positions = Dnn.adjust_to_dnn(states)
        feed_dict = {
            self.x: x,
            self.is_training: False,
            self.input_dropout_rate: 0,
            self.hidden_dropout_rate: 0
        }
        result = self.sess.run([self.values, self.policies0, self.policies1, self.policies2, self.policies3], feed_dict=feed_dict)
        pp = Dnn.unadjust_policy(result[1], result[2], result[3], result[4], positions)
        return {'policy_pair':pp, 'value':result[0]}

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

def to_state(state_json):
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

    return State(np.array(fld), agent_pos, res_turn)

    
class TrainingData:
    def __init__(self, state, visit_counts, q, z):
        self.state = state
        self.visit_counts = visit_counts
        self.q = q
        self.z = z
    def to_json(self):
        js = collections.OrderedDict()
        js['state'] = self.state.to_json()
        js['q'] = "{0:.16f}".format(self.q)
        js['z'] = "{0:.16f}".format(self.z)
        v = [["" for j in range(Move.max_int())] for i in range(2)]
        for i in range(2):
            for j in range(Move.max_int()):
                v[i][j] = str(self.visit_counts[i][j])
        js['visitCount'] = v
        return js

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
