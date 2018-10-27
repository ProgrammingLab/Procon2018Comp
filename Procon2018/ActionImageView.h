#pragma once
#include "ActionImage.h"
#include "Playground.h"

namespace Procon2018 {
class ActionImageView {
public:
	ActionImageView();
	~ActionImageView();
	void upd(const std::array<OptAction, 4> &actions);

private:

	ActionImage actionImageOfAgent1, actionImageOfAgent2;
	void draw();
	void updActionImageView(const std::array<OptAction, 4> &actions);

};
}

