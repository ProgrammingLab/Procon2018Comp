#pragma once
#include "Field.h"

namespace Procon2018 {


using Policy = std::array<double, PlayerMove::IntCount()>;
using IntMoves = int;

class Node {
private:

	std::array<std::array<double, PlayerMove::IntCount()>, 2> m_w;
	
	std::array<std::array<int, PlayerMove::IntCount()>, 2> m_count;
	
	int m_countSum;
	
	std::array<Policy, 2> m_policyPair;
	
	std::map<int, Node> m_next;
	
public:
	
	static IntMoves ToInt(const std::pair<PlayerMove, PlayerMove> &movePair);

	static IntMoves ToInt(PlayerMove m0, PlayerMove m1);

	static std::pair<PlayerMove, PlayerMove> ToMoves(IntMoves i);

	Node(const std::array<Policy, 2> &policyPair);

	IntMove decideMove(PlayerId playerId, const Field &field) const;

	IntMoves decideMoves(const Field &field) const;

	void backup(IntMoves i, double v);
};


}