# coding: utf-8

from tcpLib import *
import sys
import os
import argparse
import json
import subprocess
import shutil

parser = argparse.ArgumentParser()
parser.add_argument('learner_ip', \
    action='store', \
    nargs=None, \
    const=None, \
    default=None, \
    type=str, \
    choices=None, \
    help='IP address of learner', \
    metavar=None)
args = parser.parse_args()

while True:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    while True:
        try:
            sock.connect((args.learner_ip, 54217))
            break
        except:
            continue
    header = json.loads(myreceive_with_sign(sock).decode('utf-8'))
    game_count = int(header['gameCount'])
    ckpt = int(header['checkpoint'])
    model_dir = './model/currentModel'
    model_path = model_dir + ('/ckpt=%d' % ckpt)
    if os.path.isdir(model_dir):
        shutil.rmtree(model_dir)
    os.makedirs(model_dir, exist_ok=True)
    for i in range(len(header['modelFiles'])):
        b = myreceive_with_sign(sock)
        with open(model_dir + '/' + header['modelFiles'][i], 'wb') as f:
            f.write(b)
    
    proc = subprocess.Popen(['python', model_dir + '/dnnServer.py', './' + model_path], stdout=subprocess.PIPE)
    subprocess.run([model_dir + '/SelfPlay_win.exe', str(game_count), args.learner_ip])
    proc.kill()
