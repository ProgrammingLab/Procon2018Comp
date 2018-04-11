#include "FieldView.h"


namespace Procon2018 {


FieldView::FieldView(const s3d::RectF & viewport)
: m_v(viewport)
, m_fld()
, m_gridSize(std::min(m_v.w/m_fld.w(), m_v.h/m_fld.h()))
, m_gridScoreFont((int)(m_gridSize*0.5)) {
	
}

void FieldView::update() {
	m_v.draw(s3d::Palette::Gray);
	for (int yi = 0; yi < m_fld.h(); yi++) {
		for (int xi = 0; xi < m_fld.w(); xi++) {
			const Grid& grid = m_fld.grid({xi, yi});
			s3d::Vec2 p(xi*m_gridSize, yi*m_gridSize);
			s3d::Vec2 c(m_gridSize/2, m_gridSize/2);
			s3d::Color color = s3d::Palette::White;
			if (grid.color) switch (grid.color.value()) {
			case TeamId::A:
				color = s3d::Palette::Blue;
				break;
			case TeamId::B:
				color = s3d::Palette::Red;
				break;
			default:
				break;
			}
			s3d::RectF(p.x, p.y, m_gridSize)
				.draw(color)
				.drawFrame(1, s3d::Palette::Black);
			m_gridScoreFont(grid.score)
				.draw(s3d::Arg::center(p + c), s3d::Palette::Black);
		}
	}
	for (int i = 0; i < 4; i++) {
		s3d::Point pp = m_fld.playerPos((PlayerId)i);
		TeamId team = m_fld.teamOf((PlayerId)i);
		s3d::Color color = team ? s3d::Palette::Red : s3d::Palette::Blue;
		s3d::Vec2 p(pp.x*m_gridSize + m_gridSize/2, pp.y*m_gridSize + m_gridSize/2);
		s3d::Circle(p, m_gridSize/2).draw(color).drawFrame(1, s3d::Palette::Black);
	}
}


}