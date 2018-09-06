#include "Visualizer.h"


namespace Procon2018 {


Visualizer::Visualizer()
: m_fv(s3d::Window::ClientRect(), Field::RandomState())
, m_sw()
, m_interval(1) {
	m_sw.start();
}

void Visualizer::update() {
	auto randomAction = []() {
		ActionType type = (ActionType)s3d::Random(1);
		Direction8 dir = (Direction8)s3d::Random(7);
		return Action(type, dir);
	};

	if (m_sw.sF() >= m_interval.count()) {
		m_sw.set((s3d::SecondsF)m_sw.sF() - m_interval);
		Action a0 = randomAction(),
			   a1 = randomAction(),
			   b0 = randomAction(),
			   b1 = randomAction();
		m_fv.forward(a0, a1, b0, b1);
	}
	
	m_fv.update();
}


}