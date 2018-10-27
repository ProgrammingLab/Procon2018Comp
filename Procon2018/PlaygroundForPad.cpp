#include "stdafx.h"
#include "PlaygroundForPad.h"


namespace Procon2018 {


PlaygroundForPad::PlaygroundForPad(const s3d::RectF & viewport, PlayerId padPlayer)
	: PlaygroundForPad(viewport, SP<AI>(), SP<AI>(), padPlayer) {}

PlaygroundForPad::PlaygroundForPad(const s3d::RectF & viewport, SP<AI> ai0, SP<AI> ai1, PlayerId padPlayer)
	: PlaygroundForPad(viewport, ai0, ai1, Field::RandomState(true), padPlayer) {
}

PlaygroundForPad::PlaygroundForPad(const s3d::RectF & viewport, SP<AI> ai0, SP<AI> ai1, const Field & fld, PlayerId padPlayer)
	: FieldView(viewport, fld)
	, m_actions()
	, m_ai{ ai0, ai1 }
	, m_hiddenAI(false)
	, m_cursor(0, 0)
	, m_ltFlg(false)
	, m_rtFlg(false)
	, m_padPlayer(padPlayer)
	, m_duration(0) {
	if (ai0) {
		ai0->init(m_fld, PlayerId::A);
		ai0->forward({});
	}
	if (ai1) {
		ai1->init(m_fld, PlayerId::B);
		ai1->forward({});
	}
}

void PlaygroundForPad::update() {
	FieldView::update();

	auto controller = s3d::XInput(0);
	Point pressedVec = [&] {
		Point ret(0, 0);
		bool rf = controller.buttonRight.pressed();
		bool lf = controller.buttonLeft.pressed();
		bool df = controller.buttonDown.pressed();
		bool uf = controller.buttonUp.pressed();
		if (rf) ret += Point(1, 0);
		if (lf) ret += Point(-1, 0);
		if (df) ret += Point(0, 1);
		if (uf) ret += Point(0, -1);
		return ret;
	}();
	Point downVec = [&] {
		Point ret(0, 0);
		bool rf = controller.buttonRight.down();
		bool lf = controller.buttonLeft.down();
		bool df = controller.buttonDown.down();
		bool uf = controller.buttonUp.down();
		if (controller.buttonRight.down()) ret += Point(1, 0);
		if (controller.buttonLeft.down()) ret += Point(-1, 0);
		if (controller.buttonDown.down()) ret += Point(0, 1);
		if (controller.buttonUp.down()) ret += Point(0, -1);
		if (pressedVec == Point(0, 0)) {
			m_duration = 0;
			m_highspeedSW.reset();
		}
		else {
			m_duration += m_highspeedSW.sF();
			m_highspeedSW.restart();
			if (m_duration > 0.3) {
				m_duration -= 0.06;
				ret += pressedVec;
			}
		}
		return ret;
	}();
	auto vecToDir = [&](const Point &vec) {
		for (int dir = 0; dir < 8; dir++) {
			if (Neighbour8((Direction8)dir) == vec)
				return std::make_optional((Direction8)dir);
		}
		return std::optional<Direction8>();
	};
	auto drawGridShadow = [&](const Point &p) {
		s3d::RectF(m_v.tl() + s3d::Vec2(p.x*m_gridSize, p.y*m_gridSize), m_gridSize)
			.drawShadow({ 0, 0 }, 50, 7, s3d::Color(s3d::Palette::Gray, 180));
	};
	auto actionColor = [](ActionType t) {
		if (t == ActionType::Remove) return s3d::Palette::Magenta;
		return s3d::Palette::Darkgray;
	};
	auto drawAction = [&](s3d::Line l, ActionType type) {
		l.drawArrow(5, { 10, 10 }, actionColor(type));
	};
	if (m_controlledAgent) {
		s3d::Vec2 base = gridCenter(m_fld.agentPos(*m_controlledAgent));
		s3d::Color playerColor = m_fld.playerOf(*m_controlledAgent) == PlayerId::A ? s3d::Palette::Blue : s3d::Palette::Red;
		s3d::RectF(m_v.tl() + s3d::Vec2(m_cursor.x*m_gridSize, m_cursor.y*m_gridSize), m_gridSize)
			.drawShadow({ 0, 0 }, m_gridSize*5, 10, s3d::Color(playerColor, 100));
		drawAction(s3d::Line(base, gridCenter(m_cursor + pressedVec)), ActionType::Move);
		if (controller.buttonA.down() || controller.buttonB.down()) {
			ActionType aType = controller.buttonB.down() ? ActionType::Remove : ActionType::Move;
			if (auto dir = vecToDir(pressedVec)) {
				m_actions[(int)*m_controlledAgent] = Action(aType, *dir);
			}
			m_controlledAgent.reset();
		}
	}
	else {
		auto selectedAgent = [&] {
			for (int i = 0; i < 4; i++) {
				AgentId aId = (AgentId)i;
				if (m_cursor == m_fld.agentPos(aId)) {
					m_actions[i].reset();
					return std::optional<AgentId>(aId);
				}
			}
			return std::optional<AgentId>();
		};
		PlayerId nPadPlayer = (PlayerId)(1 - (int)m_padPlayer);
		AgentId a0 = (AgentId)(2*(int)m_padPlayer + 0);
		AgentId a1 = (AgentId)(2*(int)m_padPlayer + 1);
		AgentId a2 = (AgentId)(2*(int)nPadPlayer + 0);
		AgentId a3 = (AgentId)(2*(int)nPadPlayer + 1);
		if (controller.buttonLB.down()) m_cursor = m_fld.agentPos(a0);
		if (controller.buttonRB.down()) m_cursor = m_fld.agentPos(a2);
		if (controller.leftTrigger > 0.5) {
			if (m_ltFlg == false) m_cursor = m_fld.agentPos(a1);
			m_ltFlg = true;
		}
		else m_ltFlg = false;
		if (controller.rightTrigger > 0.5) {
			if (m_rtFlg == false) m_cursor = m_fld.agentPos(a3);
			m_rtFlg = true;
		}
		else m_rtFlg = false;
		if (controller.buttonA.down()) {
			m_controlledAgent = selectedAgent();
		}
		else if (controller.buttonY.down() && !selectedAgent()) {
			Field newFld = m_fld;
			auto color = m_fld.grid(m_cursor).color;
			if (!color) color = PlayerId::A;
			else if (*color == PlayerId::A) color = PlayerId::B;
			else color.reset();
			newFld.setColor(m_cursor, color);
			transit(newFld);
			for (int i = 0; i < 2; i++) {
				if (m_ai[i]) m_ai[i]->init(newFld, (PlayerId)i);
			}
		}
		if (!m_fld.outOfField(m_cursor + downVec))
			m_cursor += downVec;
		drawGridShadow(m_cursor);
	}


	if ((s3d::KeyBackspace.down() || controller.buttonBack.down()) && m_old) {
		bool possible = true;
		for (int i = 0; i < 2; i++) {
			if (m_ai[i] && !m_ai[i]->getNextMoveWithCache()) possible = false;
		}
		if (possible) {
			for (int i = 0; i < 2; i++) {
				if (m_ai[i]) {
					m_ai[i]->init(*m_old, (PlayerId)i);
					m_ai[i]->forward({});
				}
			}
			m_actions = {};
			goBack();
		}
	}

	if (m_fld.isEnd()) return;

	std::array<std::vector<Point>, 4> pathGuide;
	bool forwards = s3d::KeyEnter.down() || controller.buttonStart.down();
	bool validInput[4] = {};
	for (int i = 0; i < 2; i++) {
		if (m_ai[i] == nullptr) {
			validInput[2*i] = validInput[2*i + 1] = true;
			continue;
		}
		auto move = m_ai[i]->checkNextMove();
		if (!m_ai[i]->getNextMoveWithCache()) {
			forwards = false;
		}
		else {
			if (i == (int)m_padPlayer) {
				pathGuide = m_ai[i]->pathGuide();
			}
		}
		if (move) {
			m_actions[2*i] = move->a0;
			m_actions[2*i + 1] = move->a1;
		}
	}
	if (m_ai[0] && m_ai[1] && m_ai[0]->getNextMoveWithCache() && m_ai[1]->getNextMoveWithCache())
		forwards = true;

	auto toGridPos = [&](s3d::Vec2 vec) {
		s3d::Vec2 pos = vec - m_v.tl();
		return Point((int)(pos.x / m_gridSize), (int)(pos.y / m_gridSize));
	};

	s3d::Vec2 mousePos = s3d::Cursor::PosF();

	/*if (s3d::MouseL.down() || s3d::MouseR.down()) {
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
			.drawShadow({ 0, 0 }, 20, 7, s3d::Color(color, 100));

		if (!l && !r) return;
		m_actions[(int)m_dragState->selected] = std::make_optional(Action(m_dragState->type, dir));
		m_dragState.reset();
	} ();*/

	for (int i = 0; i < 4; i++) {
		s3d::LineString ls;
		AgentId aId = (AgentId)i;
		s3d::Color playerColor = m_fld.playerOf(aId) == PlayerId::A ? s3d::Palette::Blue : s3d::Palette::Red;
		playerColor = s3d::Color(playerColor, 100);
		for (int j = 0; j < pathGuide[i].size(); j++) {
			ls.push_back(gridCenter(pathGuide[i][j]));
		}
		ls.draw(3, playerColor);
	}

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

void PlaygroundForPad::setHiddenAI(bool value) {
	m_hiddenAI = value;
}


}