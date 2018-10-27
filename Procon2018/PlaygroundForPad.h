#pragma once
#include "Shared/Util.h"
#include "FieldView.h"
#include "AI.h"
#include "QRReader.h"
#include"Panel.h"


namespace Procon2018 {


class PlaygroundForPad : FieldView {
private:

	std::array<OptAction, 4> m_actions;

	Point m_cursor;

	std::optional<AgentId> m_controlledAgent;

	SP<AI> m_ai[2];

	bool m_hiddenAI;

	bool m_ltFlg;

	bool m_rtFlg;

	PlayerId m_padPlayer;

	double m_duration;

	s3d::Stopwatch m_highspeedSW;

public:

	PlaygroundForPad(const s3d::RectF &viewport, PlayerId padPlayer);

	PlaygroundForPad(const s3d::RectF &viewport, SP<AI> ai0, SP<AI> ai1, PlayerId padPlayer);

	PlaygroundForPad(const s3d::RectF &viewport, SP<AI> ai0, SP<AI> ai1, const Field &fld, PlayerId padPlayer);

	void update();

	void setHiddenAI(bool value);

	std::array<OptAction, 4> getActions() { return m_actions; }
};


}