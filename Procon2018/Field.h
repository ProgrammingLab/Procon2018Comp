#pragma once

namespace Procon2018 {


enum PlayerId {
	A = 0,
	B = 1,
};


enum AgentId {
	A0 = 0,
	A1 = 1,
	B0 = 2,
	B1 = 3,
};


enum ActionType {
	Move = 0,   // �ړ�
	Remove = 1, // �^�C������
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

	s3d::Point m_agent[4];

public:

	Field();

	int maxTurn() const;

	int turn() const;

	int w() const;
	
	int h() const;

	const Grid& grid(const s3d::Point &pos) const;

	const s3d::Point& playerPos(AgentId agentId) const;

	PlayerId teamOf(AgentId agentId) const;

	bool outOfField(const s3d::Point &pos) const;

	std::pair<int, int> calcScore() const;

	bool checkValid(AgentId agentId, const Action &a) const;

	// ���߂����s��, �^�[����i�߂�
	// �K��^�[�����߂̏ꍇ��false��Ԃ�
	bool forward(const OptAction &a0,
				 const OptAction &a1,
				 const OptAction &b0,
				 const OptAction &b1);

	// �s���Ȗ��߂���ł������false
	bool checkAllValid(const OptAction &a0,
					   const OptAction &a1,
					   const OptAction &b0,
					   const OptAction &b1) const;

	bool forward(const PlayerMove &m0, const PlayerMove &m1);

	bool checkAllValid(const PlayerMove &m0, const PlayerMove &m1) const;

	bool checkAllValid(PlayerId playerId, const PlayerMove &m) const;
};


}