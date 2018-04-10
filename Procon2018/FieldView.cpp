#include "FieldView.h"


namespace Procon2018 {


FieldView::FieldView(const s3d::RectF & viewport)
: m_v(viewport)
, m_fld()
, m_gridSize(std::min(m_v.w/m_fld.w(), m_v.h/m_fld.h()))
, m_gridScoreFont(m_gridSize*0.5) {
	
}

void FieldView::update() {
	m_v.draw(s3d::Palette::Gray);
	for (int yi = 0; yi < m_fld.h(); yi++) {
		for (int xi = 0; xi < m_fld.w(); xi++) {
			s3d::Vec2 p(xi*m_gridSize, yi*m_gridSize);
			s3d::Vec2 c(m_gridSize/2, m_gridSize/2);
			s3d::RectF(p.x, p.y, m_gridSize)
				.draw(s3d::Palette::White)
				.drawFrame(1, s3d::Palette::Black);
			m_gridScoreFont(m_fld.grid({xi, yi}).score)
				.draw(s3d::Arg::center(p + c), s3d::Palette::Black);
		}
	}
}


}