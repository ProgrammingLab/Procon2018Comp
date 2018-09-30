#pragma once
#include "Shared/Field.h"
#include "Shared/Util.h"

namespace Procon2018 {


class FieldView {
private:

	s3d::Color m_oldColor[Field::MAX_H][Field::MAX_W];

	Point m_oldPlayerPos[4];

protected:

	s3d::RectF m_v;

	Field m_fld;

	double m_gridSize;

	s3d::Font m_gridScoreFont;

	s3d::SecondsF m_animationTime;

	s3d::Stopwatch m_sw;


	s3d::Color gridColor(const Point &pos) const;

	s3d::Vec2 gridCenter(const Point &pos) const;

public:
	
	// viewport: 描画範囲
	FieldView(const s3d::RectF &viewport, const Field &fld);

	Field field() const;

	void update();

	bool forward(const std::optional<const Action> &a0,
				 const std::optional<const Action> &a1,
				 const std::optional<const Action> &b0,
				 const std::optional<const Action> &b1);

	void transit(const Field &next);
};


}