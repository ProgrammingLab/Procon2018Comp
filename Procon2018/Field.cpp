#include "Field.h"

using namespace s3d;


namespace Procon2018 {


const Grid& Field::grid(const Point &pos) const {
	return field[pos.y][pos.x];
}

const Point& Field::playerPos(PlayerId playerId) const {
	return player[playerId];
}

TeamId Field::teamOf(PlayerId playerId) const {
	return playerId < 2 ? TeamId::A : TeamId::B;
}

bool Field::outOfField(const Point &pos) const {
	return pos.x < 0 || w <= pos.x || pos.y < 0 || h <= pos.y;
}

std::pair<int, int> Field::calcScore() const {
	// TODO:
}

bool Field::forward(const std::optional<Action>& a0, const std::optional<Action>& a1, const std::optional<Action>& b0, const std::optional<Action>& b1) {
	// TODO:
}

bool Field::forwardable(const std::optional<Action>& a0, const std::optional<Action>& a1, const std::optional<Action>& b0, const std::optional<Action>& b1) const {
	// TODO:
}


}