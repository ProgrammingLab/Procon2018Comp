#include "Node.h"

namespace Procon2018 {


IntMoves Node::ToInt(const std::pair<PlayerMove, PlayerMove>& movePair) {
	return (IntMoves)(movePair.first.toInt()*PlayerMove::IntCount() + movePair.second.toInt());
}

IntMoves Node::ToInt(PlayerMove m0, PlayerMove m1) {
	return (IntMoves)(m0.toInt()*PlayerMove::IntCount() + m1.toInt());
}

std::pair<PlayerMove, PlayerMove> Node::ToMoves(IntMoves i) {
	PlayerMove fst = PlayerMove::FromInt(i/PlayerMove::IntCount());
	PlayerMove scd = PlayerMove::FromInt(i%PlayerMove::IntCount());
	return std::pair<PlayerMove, PlayerMove>(fst, scd);
}

Node::Node(const PolicyPair& policyPair)
: m_w()
, m_count()
, m_countSum(0)
, m_policyPair(policyPair)
, m_next() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < PlayerMove::IntCount(); j++) {
			m_w[i][j] = 0.0;
			m_count[i][j] = 0;
		}
	}
}

IntMove Node::decideMove(PlayerId playerId, const Field & field, bool kiresou) const {
	double max = -1e10;
	IntMove res = -1;
	for (IntMove i = 0; i < PlayerMove::IntCount(); i++) {
		PlayerMove m = PlayerMove::FromInt(i);
		if (!field.checkAllValid(playerId, m))
			continue;
		double c = 12.0;
		if (kiresou && field.isBad(playerId, m))
			continue;
		double w = m_w[(int)playerId][i];
		double n = m_count[(int)playerId][i];
		double q = (n > 0 ? w/n : 0);
		double p = m_policyPair[(int)playerId][i];
		double u = c * p * std::sqrt(m_countSum + 0.1) / (1 + n);
		if (!m.a0 || !m.a1) u = 0.0;
		if (res == -1 || max < q + u) {
			max = q + u;
			res = i;
		}
	}
	if (res == -1) {
		return decideMove(playerId, field, false);
	}
	return res;
}

IntMoves Node::decideMoves(const Field & field, bool kiresou) const {
	int	i0 = decideMove((PlayerId)0, field, kiresou);
	int i1 = decideMove((PlayerId)1, field, kiresou);
	return (IntMoves)(i0*PlayerMove::IntCount() + i1);
}

void Node::backup(IntMoves i, double v) {
	IntMove i0 = i/PlayerMove::IntCount();
	IntMove i1 = i%PlayerMove::IntCount();
	m_countSum++;
	m_count[0][i0]++;
	m_count[1][i1]++;
	m_w[0][i0] += v;
	m_w[1][i1] += -v;
}

}