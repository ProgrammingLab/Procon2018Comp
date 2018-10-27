#pragma once
#include "ActionImage.h"
#include "Playground.h"

namespace Procon2018 {
class ActionImageView {
public:
	ActionImageView(PlayerId pId);
	~ActionImageView();
	void upd(const std::array<OptAction, 4> &actions);

private:

	ActionImage actionImageOfAgent1, actionImageOfAgent2;
	PlayerId pId;
	void draw();
	void updActionImageView(const std::array<OptAction, 4> &actions);

};
}

