#pragma once
#include "Field.h"


namespace Procon2018 {


class FieldView {
private:

	s3d::RectF m_v;

	Field m_fld;

	double m_gridSize;

	s3d::Font m_gridScoreFont;

public:

	FieldView(const s3d::RectF &viewport);

	void update();
};


}