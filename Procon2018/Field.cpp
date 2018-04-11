#include "Field.h"
#include "stdafx.h"

using namespace s3d;


namespace Procon2018 {


Field::Field()
: m_maxTurn(10000000)
, m_turn(0)
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

bool Field::checkValid(PlayerId playerId, const Action & a) const {
	if (a.type == ActionType::Move) {
		s3d::Point next = m_player[playerId] + Neighbour8(a.dir);
		if (outOfField(next)) return false;
		if (auto &c = m_field[next.y][next.x].color)
			if (c.value() != teamOf(playerId)) return false;
		return true;
	}
	if (a.type == ActionType::Remove) {
		s3d::Point target = m_player[playerId] + Neighbour8(a.dir);
		if (outOfField(target)) return false;
		if (auto &c = m_field[target.y][target.x].color)
			if (c.value() != teamOf(playerId)) return true;
		return false;
	}
	throw "エッ";
}

bool Field::forward(const std::optional<const Action>& a0,
					const std::optional<const Action>& a1,
					const std::optional<const Action>& b0,
					const std::optional<const Action>& b1) {
	if (m_turn >= m_maxTurn) return false;

	const std::optional<const Action>* v[4] = {&a0, &a1, &b0, &b1 };
	s3d::Point pos[4];
	std::optional<s3d::Point> target[4];
	for (int i = 0; i < 4; i++) {
		pos[i] = m_player[i];
		if (!*v[i]) continue; // 停留
		const Action &a = v[i]->value();
		if (!checkValid((PlayerId)i, a)) continue; // 不正
		s3d::Point p = m_player[i] + Neighbour8(a.dir);
		if (a.type == ActionType::Move)
			pos[i] = p;
		else if (a.type == ActionType::Remove) {
			target[i] = p;
		}
		else throw ("エッ");
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < i; j++) {
			// 行き先が被ったので無効
			if (pos[j] == pos[i]) {
				pos[i] = m_player[i];
				pos[j] = m_player[j];
			}
			// 除去先が被ったので無効
			if ((target[i] && target[j]) && target[j].value() == target[i].value()) {
				target[i].reset();
				target[j].reset();
			}
		}
	}

	// 除去処理
	for (int i = 0; i < 4; i++) {
		if (!target[i]) continue;
		m_field[target[i].value().y][target[i].value().x].color.reset();
	}
	
	// 塗り絵処理
	for (int i = 0; i < 4; i++) {
		m_player[i] = pos[i];
		m_field[pos[i].y][pos[i].x].color = teamOf((PlayerId)i);
	}

	m_turn++;
	return true;
}

bool Field::checkAllValid(const std::optional<const Action>& a0,
						  const std::optional<const Action>& a1,
						  const std::optional<const Action>& b0,
						  const std::optional<const Action>& b1) const {
	const std::optional<const Action>* v[4] = {&a0, &a1, &b0, &b1 };
	for (int i = 0; i < 4; i++) {
		if (!*v[i]) continue;
		if (checkValid((PlayerId)i, v[i]->value())) return false;
	}
	return true;
}


}