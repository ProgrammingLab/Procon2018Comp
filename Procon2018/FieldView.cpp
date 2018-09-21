#include "FieldView.h"


namespace Procon2018 {


s3d::Color FieldView::gridColor(const Point & pos) const {
	const Grid &grid = m_fld.grid(pos);
	if (!grid.color) return s3d::Color(0, 0);
	if (grid.color.value() == PlayerId::A)
		return s3d::Color(s3d::Palette::Blue, 100);
	if (grid.color.value() == PlayerId::B)
		return s3d::Color(s3d::Palette::Red, 100);
	throw "エッ";
}

FieldView::FieldView(const s3d::RectF & viewport, const Field &fld)
: m_v(viewport)
, m_fld(fld)
, m_gridSize(std::min(m_v.w/m_fld.w(), m_v.h/m_fld.h()))
, m_gridScoreFont((int)(m_gridSize*0.5))
, m_animationTime(0.3)
, m_sw()
, m_oldColor()
, m_oldPlayerPos() {
	m_sw.set(m_animationTime);
}

Field FieldView::field() const {
	return m_fld;
}

void FieldView::update() {
	if (m_sw.sF() >= m_animationTime.count()) m_sw.pause();
	double t = std::min(1.0, m_sw.sF()/m_animationTime.count());

	auto drawAgent = [&](int i) {
		Point currentPos = m_fld.agentPos((AgentId)i);
		Point oldPos = m_oldPlayerPos[i];
		PlayerId team = m_fld.teamOf((AgentId)i);
		s3d::Color color = team == PlayerId::B ? s3d::Palette::Red : s3d::Palette::Blue;
		s3d::Vec2 end = m_v.tl() + s3d::Vec2(
			currentPos.x*m_gridSize + m_gridSize/2,
			currentPos.y*m_gridSize + m_gridSize/2
		);
		s3d::Vec2 start = m_v.tl() + s3d::Vec2(
			oldPos.x*m_gridSize + m_gridSize/2,
			oldPos.y*m_gridSize + m_gridSize/2
		);
		s3d::Vec2 p = s3d::EaseOut(s3d::Easing::Quart, start, end, t);
		if ((i&1) == 0)
			s3d::Circle(p, m_gridSize/3).draw(color).drawFrame(1, s3d::Palette::Black);
		else
			s3d::RectF(s3d::Arg::center(p), m_gridSize*0.9).draw(color).drawFrame(1, s3d::Palette::Black);
	};

	m_v.draw(s3d::Palette::Gray);
	for (int yi = 0; yi < m_fld.h(); yi++) {
		for (int xi = 0; xi < m_fld.w(); xi++) {
			const Grid& grid = m_fld.grid({xi, yi});
			s3d::Vec2 p = m_v.tl() + s3d::Vec2(xi*m_gridSize, yi*m_gridSize);
			s3d::Vec2 c(m_gridSize/2, m_gridSize/2);
			s3d::Color color = gridColor({xi, yi});
			s3d::Color oldColor = m_oldColor[yi][xi];
			s3d::RectF(p.x, p.y, m_gridSize)
				.draw(s3d::Palette::White)
				.draw(s3d::EaseOut(s3d::Easing::Quart, oldColor, color, t))
				.drawFrame(1, s3d::Palette::Black);

			s3d::Color scoreColor = s3d::Palette::Black;
			for (int i = 0; i < 4; i++) {
				if (Point(xi, yi) == m_fld.agentPos((AgentId)i)) {
					drawAgent(i);
					scoreColor = s3d::Palette::White;
				}
			}

			m_gridScoreFont(grid.score)
				.draw(s3d::Arg::center(p + c), scoreColor);
		}
	}
	for (int i = 0; i < 4; i++) {
	}
}

bool FieldView::forward(const std::optional<const Action>& a0,
						const std::optional<const Action>& a1,
						const std::optional<const Action>& b0,
						const std::optional<const Action>& b1) {
	for (int yi = 0; yi < m_fld.h(); yi++) {
		for (int xi = 0; xi < m_fld.w(); xi++) {
			m_oldColor[yi][xi] = gridColor({xi, yi});
		}
	}
	for (int i = 0; i < 4; i++) {
		m_oldPlayerPos[i] = m_fld.agentPos((AgentId)i);
	}
	bool ret = m_fld.forward(a0, a1, b0, b1);
	m_sw.restart();
	return ret;
}

void FieldView::transit(const Field & next) {
	for (int yi = 0; yi < m_fld.h(); yi++) {
		for (int xi = 0; xi < m_fld.w(); xi++) {
			m_oldColor[yi][xi] = gridColor({xi, yi});
		}
	}
	for (int i = 0; i < 4; i++) {
		m_oldPlayerPos[i] = m_fld.agentPos((AgentId)i);
	}
	m_fld = next;
	m_sw.restart();
}


}