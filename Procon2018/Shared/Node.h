#pragma once
#include "Util.h"
#include "Field.h"

namespace Procon2018 {


using IntMoves = int;
using Policy = std::array<double, PlayerMove::IntCount()>;
using PolicyPair = std::array<Policy, 2>; //2プレイヤー分の方策
using VisitCount = std::array<std::array<int, PlayerMove::IntCount()>, 2>;


class Node {
public: // 強引なpublicは天才プログラマの特権 #アゲてこうぜ #副作用楽勝 #privateは陰キャ #実質構造体

	std::array<std::array<double, PlayerMove::IntCount()>, 2> m_w;
	
	VisitCount m_count;
	
	int m_countSum;
	
	PolicyPair m_policyPair;
	
	std::map<IntMoves, SP<Node>> m_next;



	
	static IntMoves ToInt(const std::pair<PlayerMove, PlayerMove> &movePair);

	static IntMoves ToInt(PlayerMove m0, PlayerMove m1);

	static std::pair<PlayerMove, PlayerMove> ToMoves(IntMoves i);

	// 実体化するのは訪れる時だけ (展開時は実体化まではしない)
	Node(const PolicyPair &policyPair);

	IntMove decideMove(PlayerId playerId, const Field &field, bool kiresou = false) const;

	IntMoves decideMoves(const Field &field, bool kiresou = false) const;

	void backup(IntMoves i, double v);
};


}