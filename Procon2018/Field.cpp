#include "Field.h"

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

bool Field::forward(const std::optional<Action>& a0, const std::optional<Action>& a1, const std::optional<Action>& b0, const std::optional<Action>& b1) {
	// TODO:
	return false;
}

bool Field::forwardable(const std::optional<Action>& a0, const std::optional<Action>& a1, const std::optional<Action>& b0, const std::optional<Action>& b1) const {
	// TODO:
	return false;
}


}