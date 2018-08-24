#pragma once
#include "Util.h"

namespace Procon2018 {


enum class PlayerId {
	A = 0,
	B = 1,
};


enum class AgentId {
	A_0 = 0,
	A_1 = 1,
	B_0 = 2,
	B_1 = 3,
};


enum class ActionType {
	Move = 0,   // 移動
	Remove = 1, // タイル除去
};


struct Grid {

	int score;
	
	std::optional<PlayerId> color;
};


using IntAction = int;
struct Action {

	ActionType type;

	Direction8 dir;

	Action(ActionType type, Direction8 dir);

	static IntAction ToInt(const std::optional<Action> &a);

	static std::optional<Action> FromInt(IntAction i);

	constexpr static int IntCount() { return 17; }
};
using OptAction = std::optional<Action>;


using IntMove = int;
struct PlayerMove {

	OptAction a0;

	OptAction a1;

	PlayerMove();

	PlayerMove(const OptAction &a0, const OptAction &a1);

	IntMove toInt() const;

	static PlayerMove FromInt(IntMove i);

	constexpr static int IntCount() { return Action::IntCount()*Action::IntCount(); }
};


class Field {
public:

	static const int MAX_W = 12, MAX_H = 12;

protected:

	int m_maxTurn, m_turn, m_w, m_h;

	Grid m_field[MAX_H][MAX_W];

	Point m_agent[4];

public:

	Field();

	int maxTurn() const;

	int turn() const;

	int w() const;
	
	int h() const;

	const Grid& grid(const Point &pos) const;

	const Point& playerPos(AgentId agentId) const;

	PlayerId teamOf(AgentId agentId) const;

	bool outOfField(const Point &pos) const;

	std::pair<int, int> calcScore() const;

	bool checkValid(AgentId agentId, const Action &a) const;

	// 命令を実行し, ターンを進める
	// 規定ターン超過の場合はfalseを返す
	bool forward(const OptAction &a0,
				 const OptAction &a1,
				 const OptAction &b0,
				 const OptAction &b1);

	// 不正な命令が一つでもあればfalse
	bool checkAllValid(const OptAction &a0,
					   const OptAction &a1,
					   const OptAction &b0,
					   const OptAction &b1) const;

	bool forward(const PlayerMove &m0, const PlayerMove &m1);

	bool checkAllValid(const PlayerMove &m0, const PlayerMove &m1) const;

	bool checkAllValid(PlayerId playerId, const PlayerMove &m) const;
};


}