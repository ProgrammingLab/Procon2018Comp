# coding: utf-8

import subprocess
import argparse

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
parser.add_argument('en_route', \
    action='store', \
    nargs=None, \
    const=None, \
    default=None, \
    type=int, \
    choices=None, \
    help='0 or 1', \
    metavar=None)
args = parser.parse_args()
client_count = args.client_count
game_count = args.client_game_count
en_route = args.en_route

if en_route == 1:
    subprocess.run(['python', 'learner.py', str(client_count), str(game_count), '0'])
while True:
    subprocess.run(['python', 'learner.py', str(client_count), str(game_count), '1'])
