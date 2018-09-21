#include "stdafx.h"
#include "TrainDataVisualizer.h"


namespace Procon2018 {


TrainDataVisualizer::TrainDataVisualizer(const s3d::RectF &v, std::string gameLogDir)
: m_v(v)
, m_fv()
, m_states()
, m_turn(0) {
	using namespace boost::property_tree;
	int i = 0;
	while (true) {
		std::ifstream ifs(gameLogDir + "/" + std::to_string(i) + ".json");

		if (!ifs.is_open()) break;
		ptree pt;
		read_json(ifs, pt);
		m_states.push_back(Field::FromPTree(pt.get_child("state")));
		i++;
	}
	if (m_states.size() == 0) throw "log not found";
	m_fv = std::make_optional(FieldView(m_v, m_states[0]));
}

void TrainDataVisualizer::update() {
	if (m_turn > 0 && s3d::KeyLeft.down()) {
		m_turn--;
		m_fv->transit(m_states[m_turn]);
		std::cout << "turn: " << m_turn << std::endl;
		auto p = m_states[m_turn].calcScore();
		std::cout << "score: " << p.first - p.second << std::endl;
	}
	if (m_turn < (int)m_states.size() - 1 && s3d::KeyRight.down()) {
		m_turn++;
		m_fv->transit(m_states[m_turn]);
		std::cout << "turn: " << m_turn << std::endl;
		auto p = m_states[m_turn].calcScore();
		std::cout << "score: " << p.first - p.second << std::endl;
	}
	m_fv->update();
}


TrainDataVisualizer::~TrainDataVisualizer() {}


}