#pragma once

namespace Procon2018 {


enum TeamId {
	A = 0,
	B = 1,
};


enum PlayerId {
	A0 = 0,
	A1 = 1,
	B0 = 2,
	B1 = 3,
};


enum ActionType {
	Move = 0,   // 移動
	Remove = 1, // タイル除去
};


struct Grid {

	int score;
	
	std::optional<TeamId> color;
};


struct Action {

	ActionType type;
	
	PlayerId playerId;

	Direction8 dir;
};


class Field {
public:

	static const int MAX_W = 12, MAX_H = 12;

protected:

	int m_maxTurn, m_turn, m_w, m_h;

	Grid m_field[MAX_H][MAX_W];

	s3d::Point m_player[4];

public:

	Field();

	int maxTurn() const;

	int turn() const;

	int w() const;
	
	int h() const;

	const Grid& grid(const s3d::Point &pos) const;

	const s3d::Point& playerPos(PlayerId playerId) const;

	TeamId teamOf(PlayerId playerId) const;

	bool outOfField(const s3d::Point &pos) const;

	std::pair<int, int> calcScore() const;
	// 命令を実行し, ターンを進める
	// 命令が実行できない場合はfalseを返す
	bool forward(const std::optional<Action> &a0,
				 const std::optional<Action> &a1,
				 const std::optional<Action> &b0,
				 const std::optional<Action> &b1);

	bool forwardable(const std::optional<Action> &a0,
					 const std::optional<Action> &a1,
					 const std::optional<Action> &b0,
					 const std::optional<Action> &b1) const;
};


}