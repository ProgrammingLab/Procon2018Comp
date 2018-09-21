#pragma once
#include "Util.h"
#include "DnnClient.h"
#include "Field.h"


namespace Procon2018 {
//TODO: 通信のエラー処理
//TODO: fieldがgoDown()のはじめからゲーム終了の場合

class Mcts {
private:

	Field m_rootState;

	SP<Node> m_root;

public:

	Mcts(const Field &rootState, SP<DnnClient> dnn);
	
	~Mcts();

	Field copyRootState() const;

	SP<Node> root() const;

	bool isEnd() const;

	// fieldは変更され, 最終的に, 評価が必要な盤面になる
	// pathに, 後のbackupで使う経路が根から順に入る
	// return: fieldがゲーム終了状態ならtrue
	// nodeがnullなら何もしない
	bool goDown(SP<Node> node, Field &field, std::vector<IntMoves> &path);

	bool goDown(Field &field, std::vector<IntMoves> &path);

	// 評価結果vをバックアップ
	void backupWithExpansion(const std::vector<IntMoves> &path, double v, const PolicyPair &policyPair);

	void backup(const std::vector<IntMoves> &path, double v);

	// 自己対局を1ターン進める. 根ノードも取り替える.
	// ゲームが終了した時trueを返す. 以降呼び出されても何もしない
	// temperatureが1に近づくほど, 訪問回数に比例した確率で手を選ぶ
	// temperatureが0に近づくほど, 訪問回数の多い手をひいきする
	bool selfNext(double temperature, SP<DnnClient> dnn);

	void next(PlayerMove m1, SP<DnnClient> dnn);
};


}