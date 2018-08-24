#pragma once
#include "FieldView.h"


namespace Procon2018 {


class Visualizer {
private:

	const s3d::SecondsF m_interval;

	FieldView m_fv;

	s3d::Stopwatch m_sw;

public:

	Visualizer();

	void update();

};


}