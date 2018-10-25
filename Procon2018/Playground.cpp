#include "stdafx.h"
#include "Playground.h"


namespace Procon2018 {


Playground::Playground(const s3d::RectF & viewport)
: FieldView(viewport, Field::RandomState())
, m_actions()
, m_dragState()
, m_ai()
, m_hiddenAI(false) {
}

Playground::Playground(const s3d::RectF & viewport, SP<AI> ai0, SP<AI> ai1)
: FieldView(viewport, Field::RandomState(true))
, m_actions()
, m_dragState()
, m_ai{ai0, ai1}
, m_hiddenAI(false) {
	if (ai0) {
		ai0->init(m_fld, PlayerId::A);
		ai0->forward({});
	}
	if (ai1) {
		ai1->init(m_fld, PlayerId::B);
		ai1->forward({});
	}
}

void Playground::update() {
	FieldView::update();
	if (m_fld.isEnd()) return;

	bool forwards = s3d::KeyEnter.down();
	bool validInput[4] = {};
	for (int i = 0; i < 2; i++) {
		if (m_ai[i] == nullptr) {
			validInput[2*i] = validInput[2*i + 1] = true;
			continue;
		}
		auto move = m_ai[i]->getNextMove();
		if (!move) {
			forwards = false;
		}
		m_actions[2*i] = move->a0;
		m_actions[2*i + 1] = move->a1;
	}
	if (m_ai[0] && m_ai[1] && m_ai[0]->getNextMove() && m_ai[1]->getNextMove()) forwards = true;

	auto toGridPos = [&](s3d::Vec2 vec) {
		s3d::Vec2 pos = vec - m_v.tl();
		return Point((int)(pos.x/m_gridSize), (int)(pos.y/m_gridSize));
	};
	auto actionColor = [](ActionType t) {
		if (t == ActionType::Remove) return s3d::Palette::Magenta;
		return s3d::Palette::Darkgray;
	};
	auto drawAction = [&](s3d::Line l, ActionType type) {
		l.drawArrow(5, {10, 10}, actionColor(type));
	};

	s3d::Vec2 mousePos = s3d::Cursor::PosF();

	if (s3d::MouseL.down() || s3d::MouseR.down()) {
		Point p = toGridPos(mousePos);
		for (int i = 0; i < 4; i++) {
			if (!validInput[i]) continue;
			if (p == m_fld.agentPos((AgentId)i)) {
				DragState s;
				if (s3d::MouseL.down()) s.type = ActionType::Move;
				else if (s3d::MouseR.down()) s.type = ActionType::Remove;
				s.selected = (AgentId)i;
				m_dragState = std::make_optional(s);
				m_actions[i].reset();
				break;
			}
		}
	}
	[&]() {
		if (!m_dragState) return;
		s3d::Color color = actionColor(m_dragState->type);
		s3d::Vec2 base = gridCenter(m_fld.agentPos(m_dragState->selected));
		drawAction(s3d::Line(base, s3d::Cursor::PosF()), m_dragState->type);

		double min = 1e10;
		Direction8 dir;
		for (int i = 0; i < 8; i++) {
			s3d::Vec2 p = gridCenter(m_fld.agentPos(m_dragState->selected) + Neighbour8((Direction8)i));
			double d = mousePos.distanceFrom(p);
			if (d < min) {
				min = d;
				dir = (Direction8)i;
			}
		}
		bool l = s3d::MouseL.up(), r = s3d::MouseR.up();
		if (base.distanceFrom(mousePos) < min) {
			if (l || r) m_dragState.reset();
			return;
		}
		Point gp = m_fld.agentPos(m_dragState->selected) + Neighbour8((Direction8)dir);
		s3d::RectF(m_v.tl() + s3d::Vec2(gp.x*m_gridSize, gp.y*m_gridSize), m_gridSize)
			.drawShadow({0, 0}, 20, 7, s3d::Color(color, 100));

		if (!l && !r) return;
		m_actions[(int)m_dragState->selected] = std::make_optional(Action(m_dragState->type, dir));
		m_dragState.reset();
	} ();

	for (int i = 0; i < 4; i++) {
		if (!m_actions[i]) continue;
		if (m_hiddenAI && !validInput[i]) continue;
		Point p = m_fld.agentPos((AgentId)i);
		Point trg = p + Neighbour8(m_actions[i]->dir);
		drawAction(s3d::Line(gridCenter(p), gridCenter(trg)), m_actions[i]->type);
	}

	if (forwards && m_fld.resTurn() > 0) {
		forward(m_actions[0], m_actions[1], m_actions[2], m_actions[3]);
		std::cout << "resTurn: " << m_fld.resTurn() << std::endl;
		auto score = m_fld.calcScore();
		std::cout << "score: (blue: " << score.first << ", red: " << score.second << ")" << std::endl;
		
		if (m_fld.resTurn() > 0) {
			PlayerMove m0 = PlayerMove(m_actions[0], m_actions[1]);
			PlayerMove m1 = PlayerMove(m_actions[2], m_actions[3]);
			std::optional<std::pair<PlayerMove, PlayerMove>> moves = std::make_pair(m0, m1);
			for (int i = 0; i < 2; i++) {
				if (m_ai[i]) m_ai[i]->forward(moves);
			}
		}
		for (int i = 0; i < 4; i++) {
			m_actions[i].reset();
		}
	}
}

void Playground::setHiddenAI(bool value) {
	m_hiddenAI = value;
}


}