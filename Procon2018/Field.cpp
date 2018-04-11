#include "Field.h"
#include "stdafx.h"

using namespace s3d;


namespace Procon2018 {


Field::Field()
: m_maxTurn()
, m_turn()
, m_w(12)
, m_h(12)
, m_field()
, m_player{ {1, 1}, {11, 11}, {11, 1}, {1, 11} } {
	for (int y = 0; y < m_h; y++) for (int x = 0; x < m_w; x++) {
		m_field[y][x].score = s3d::Random(-16, 16);
		if (s3d::Random(1)) m_field[y][x].color = std::make_optional((TeamId)s3d::Random(1));
	}
}

int Field::maxTurn() const {
	return m_maxTurn;
}

int Field::turn() const {
	return m_turn;
}

int Field::w() const {
	return m_w;
}

int Field::h() const {
	return m_h;
}

const Grid& Field::grid(const Point &pos) const {
	return m_field[pos.y][pos.x];
}

const Point& Field::playerPos(PlayerId playerId) const {
	return m_player[playerId];
}

TeamId Field::teamOf(PlayerId playerId) const {
	return playerId < 2 ? TeamId::A : TeamId::B;
}

bool Field::outOfField(const Point &pos) const {
	return pos.x < 0 || m_w <= pos.x || pos.y < 0 || m_h <= pos.y;
}

std::pair<int, int> Field::calcScore() const {
	// TODO:
	return std::pair<int, int>(0, 0);
}

bool Field::forward(const std::optional<const Action>& a0,
					const std::optional<const Action>& a1,
					const std::optional<const Action>& b0,
					const std::optional<const Action>& b1) {
	if (!isForwardable(a0, a1, b0, b1)) return false;
	const std::optional<const Action>* v[4] = {&a0, &a1, &b0, &b1 };
	for (int i = 0; i < 4; i++) {
		if (!*v[i]) continue;
		const Action &a = v[i]->value();
		s3d::Point p = m_player[i] + Neighbour8(a.dir);
		if (a.type == ActionType::Move) {
			m_player[i] = p;
		}
		else if (a.type == ActionType::Remove) {
			m_field[p.y][p.x].color.reset();
		}
		else throw ("エッ");
	}
	
	// 塗り絵処理
	for (int i = 0; i < 4; i++) {
		m_field[m_player[i].y][m_player[i].x].color = teamOf((PlayerId)i);
	}

	m_turn++;
	return true;
}

bool Field::isForwardable(const std::optional<const Action>& a0,
						  const std::optional<const Action>& a1,
						  const std::optional<const Action>& b0,
						  const std::optional<const Action>& b1) const {
	auto validMove = [&](PlayerId playerId, const Action &a) {
		s3d::Point next = m_player[playerId] + Neighbour8(a.dir);
		if (auto &c = m_field[next.y][next.x].color)
			if (c.value() != teamOf(playerId)) return false;
		return true;
	};
	auto validRemove = [&](PlayerId playerId, const Action &a) {
		return !validMove(playerId, a);
	};
	auto validAction = [&](PlayerId playerId, const Action &a) {
		if (a.type == ActionType::Move) return validMove(playerId, a);
		return validRemove(playerId, a);
	};
	if (m_turn >= m_maxTurn) return false;
	const std::optional<const Action>* v[4] = {&a0, &a1, &b0, &b1 };
	s3d::Point pos[4];
	std::optional<s3d::Point> target[4];
	for (int i = 0; i < 4; i++) {
		pos[i] = m_player[i];
		if (!*v[i]) continue; // 停留
		const Action &a = v[i]->value();
		if (!validAction((PlayerId)i, a)) return false; // 単体で無効な行動
		pos[i] = m_player[i] + Neighbour8(a.dir);
		if (a.type == ActionType::Remove)
			target[i] = m_player[i] + Neighbour8(a.dir);
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < i; j++) {
			if (pos[j] == pos[i]) return false; // 行き先が被った
			if ((target[i] && target[j]) && target[j].value() == target[i].value())
				return false; // 除去先が被った
		}
	}

	return true;
}


}