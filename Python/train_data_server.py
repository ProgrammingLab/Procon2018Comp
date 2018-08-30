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
        ts = to_train_data(receive_body)

        folder_path = output_dir + '/' + str(game_id)
        os.makedirs(folder_path)

        for i in range(len(ts)):






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
