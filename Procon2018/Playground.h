#pragma once
#include "Shared/Util.h"
#include "FieldView.h"
#include "AI.h"


namespace Procon2018 {


struct DragState {
	ActionType type;
	AgentId selected;
};


class Playground : FieldView {
private:

	std::array<OptAction, 4> m_actions;

	std::optional<DragState> m_dragState;

	SP<AI> m_ai[2];

public:

	Playground(const s3d::RectF &viewport);

	Playground(const s3d::RectF &viewport, SP<AI> ai0, SP<AI> ai1);

	void update();
	
};


}