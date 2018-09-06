#pragma once

#include "FieldView.h"


namespace Procon2018 {


class TrainDataVisualizer {
private:

	s3d::RectF m_v;

	std::optional<FieldView> m_fv;

	std::vector<Field> m_states;

	int m_turn = 0;

public:

	TrainDataVisualizer(const s3d::RectF &v, std::string gameLogDir);

	void update();
	
	~TrainDataVisualizer();
};


}