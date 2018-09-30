#pragma once
#include "Shared/Util.h"
#include "FieldView.h"


namespace Procon2018 {


struct DragState {
	ActionType type;
	AgentId selected;
};


class Playground : FieldView {
private:

	std::array<OptAction, 4> m_actions;

	std::optional<DragState> m_dragState;

public:

	Playground(const s3d::RectF &viewport);

	void update();
	
};


}