#include "Field.h"


namespace Procon2018 {


Field::Field()
: m_maxTurn(10000000)
, m_turn(0)
, m_w(12)
, m_h(12)
, m_field()
, m_agent{ {1, 1}, {11, 11}, {11, 1}, {1, 11} } {
	for (int y = 0; y < m_h; y++) for (int x = 0; x < m_w; x++) {
		m_field[y][x].score = Rand::Next(-16, 16 + 1);
		if (Rand::Next(2)) m_field[y][x].color = std::make_optional((PlayerId)Rand::Next(2));
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

const Point& Field::playerPos(AgentId playerId) const {
	return m_agent[(int)playerId];
}

PlayerId Field::teamOf(AgentId playerId) const {
	return (int)playerId < 2 ? PlayerId::A : PlayerId::B;
}

bool Field::outOfField(const Point &pos) const {
	return pos.x < 0 || m_w <= pos.x || pos.y < 0 || m_h <= pos.y;
}

std::pair<int, int> Field::calcScore() const {
	std::pair<int, int> ret(0, 0);
	for (int y = 0; y < m_h; y++) for (int x = 0; x < m_w; x++) {
		const auto &c = m_field[y][x].color;
		if (!c) continue;
		if (c.value() == PlayerId::A) ret.first += m_field[y][x].score;
		else if (c.value() == PlayerId::B) ret.second += m_field[y][x].score;
		else throw "エッ";
	}

	auto specialScore = [&](PlayerId teamId) {
		constexpr Point dirPoint[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
		const int O = 1;
		bool used[Field::MAX_H + 2][Field::MAX_W + 2] = {};
		
		std::queue<Point> q;
		q.push({-1, -1});
		while (!q.empty()) {
			const Point f = q.front(); q.pop();
			if (used[f.y + O][f.x + O]) continue;
			used[f.y + O][f.x + O] = true;
			for (int i = 0; i < 4; i++) {
				const Point n = f + dirPoint[i];
				if (n.x < -1 || m_w + 1 <= n.x || n.y < -1 || m_h + 1 <= n.y)
					continue;
				if (used[n.y + O][n.x + O]) continue;
				if (!outOfField(n))
					if (auto &c = m_field[n.y][n.x].color)
						if (c.value() == teamId) continue;
				q.push(n);
			}
		}

		int ret_ = 0;
		for (int y = 0; y < m_h; y++) for (int x = 0; x < m_w; x++) {
			if (used[y + O][x + O]) continue;
			if (auto &c = m_field[y][x].color)
				if (c.value() == teamId) continue;
			ret_ += std::abs(m_field[y][x].score);
		}
		return ret_;
	};
	
	ret.first += specialScore(PlayerId::A);
	ret.second += specialScore(PlayerId::B);
	return ret;
}

bool Field::checkValid(AgentId agentId, const Action & a) const {
	if (a.type == ActionType::Move) {
		Point next = m_agent[(int)agentId] + Neighbour8(a.dir);
		if (outOfField(next)) return false;
		if (auto &c = m_field[next.y][next.x].color)
			if (c.value() != teamOf(agentId)) return false;
		return true;
	}
	if (a.type == ActionType::Remove) {
		Point target = m_agent[(int)agentId] + Neighbour8(a.dir);
		if (outOfField(target)) return false;
		if (auto &c = m_field[target.y][target.x].color)
			if (c.value() != teamOf(agentId)) return true;
		return false;
	}
	throw "エッ";
}

bool Field::forward(const OptAction& a0,
					const OptAction& a1,
					const OptAction& b0,
					const OptAction& b1) {
	if (m_turn >= m_maxTurn) return false;

	const OptAction* v[4] = {&a0, &a1, &b0, &b1 };
	Point pos[4];
	std::optional<Point> target[4];
	for (int i = 0; i < 4; i++) {
		pos[i] = m_agent[i];
		if (!*v[i]) continue; // 停留
		const Action &a = v[i]->value();
		if (!checkValid((AgentId)i, a)) continue; // 不正
		Point p = m_agent[i] + Neighbour8(a.dir);
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
				pos[i] = m_agent[i];
				pos[j] = m_agent[j];
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
		m_agent[i] = pos[i];
		m_field[pos[i].y][pos[i].x].color = teamOf((AgentId)i);
	}

	m_turn++;
	return true;
}

bool Field::checkAllValid(const OptAction& a0,
						  const OptAction& a1,
						  const OptAction& b0,
						  const OptAction& b1) const {
	const OptAction* v[4] = {&a0, &a1, &b0, &b1 };
	for (int i = 0; i < 4; i++) {
		if (!*v[i]) continue;
		if (checkValid((AgentId)i, v[i]->value())) return false;
	}
	return true;
}

bool Field::forward(const PlayerMove & m0, const PlayerMove & m1) {
	return forward(m0.a0, m0.a1, m1.a0, m1.a1);
}

bool Field::checkAllValid(const PlayerMove & m0, const PlayerMove & m1) const {
	return checkAllValid(m0.a0, m0.a1, m1.a0, m1.a1);
}

bool Field::checkAllValid(PlayerId playerId, const PlayerMove & m) const {
	AgentId i0 = (AgentId)((int)playerId*2);
	AgentId i1 = (AgentId)((int)playerId*2 + 1);
	return (!m.a0 || checkValid(i0, m.a0.value())) && (!m.a1 || checkValid(i1, m.a1.value()));
}


Action::Action(ActionType type, Direction8 dir) : type(type), dir(dir) {}

IntAction Action::ToInt(const std::optional<Action>& a) {
	if (!a) return 0;
	ActionType type = a.value().type;
	Direction8 dir = a.value().dir;
	return (type == ActionType::Remove ? 8 : 0) + dir + 1;
}

std::optional<Action> Action::FromInt(IntAction i) {
	using Opt = std::optional<Action>;
	if (i == 0) return Opt();
	if (i < 9) return Opt(Action(ActionType::Move, (Direction8)(i - 1)));
	return Opt(Action(ActionType::Remove, (Direction8)(i - 9)));
}

PlayerMove::PlayerMove() : a0(), a1() {}

PlayerMove::PlayerMove(const OptAction & a0, const OptAction & a1) : a0(a0), a1(a1) {}

int PlayerMove::toInt() const {
	return Action::ToInt(a0)*17 + Action::ToInt(a1);
}

PlayerMove PlayerMove::FromInt(IntMove i){
	return PlayerMove( Action::FromInt(i / Action::IntCount()), Action::FromInt(i % Action::IntCount()) );
}


}