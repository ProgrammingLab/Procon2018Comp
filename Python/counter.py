import os
import sys

#ディレクトリ内の全てのファイルをカウントする再帰関数
def countFiles(target_dir_name):
    #カウンタの初期化
    cnt = 0
    # 指定したパス内の全てのファイルとディレクトリを要素とするリストを返す
    target_dir_files = os.listdir(target_dir_name)
    for file in target_dir_files:
        new_target_dir_name = target_dir_name + "/" +file
        #ディレクトリか非ディレクトリで条件分岐
        if os.path.isdir(new_target_dir_name):
            #ディレクトリの場合、中に入って探索する
            cnt += countFiles(new_target_dir_name)
        else:
            #非ディレクトリの場合、数え上げを行う
            cnt += 1
    return cnt

path = sys.argv[1]
print(countFiles(path))