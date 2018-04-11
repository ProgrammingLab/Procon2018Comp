#pragma once
#include "Field.h"


namespace Procon2018 {


class FieldView {
private:

	s3d::RectF m_v;

	Field m_fld;

	double m_gridSize;

	s3d::Font m_gridScoreFont;

	s3d::SecondsF m_animationTime;

	s3d::Stopwatch m_sw;

	s3d::Color m_oldColor[Field::MAX_H][Field::MAX_W];

	s3d::Point m_oldPlayerPos[4];


	s3d::Color gridColor(const s3d::Point &pos) const;

public:

	FieldView(const s3d::RectF &viewport);

	Field field() const;

	void update();

	bool forward(const std::optional<const Action> &a0,
				 const std::optional<const Action> &a1,
				 const std::optional<const Action> &b0,
				 const std::optional<const Action> &b1);
};


}