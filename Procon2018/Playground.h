#pragma once
#include "Shared/Util.h"
#include "FieldView.h"
#include "AI.h"
#include "QRReader.h"
#include"Panel.h"


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

	bool isEditMode = false;

	bool m_hiddenAI;

public:

	Playground(const s3d::RectF &viewport);

	Playground(const s3d::RectF &viewport, SP<AI> ai0, SP<AI> ai1);

	Playground(const s3d::RectF &viewport, SP<AI> ai0, SP<AI> ai1, const Field &fld);

	void update();

	void setHiddenAI(bool value);
	
	
	void onEditMode() { isEditMode = true; }

	void offEditMode() { isEditMode = false; }

	std::array<OptAction, 4> getActions() { return m_actions; }
};


}