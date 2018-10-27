#pragma once
#include "ActionImage.h"
#include "Playground.h"

namespace Procon2018 {
class ActionImageView {
public:
	ActionImageView();
	~ActionImageView();
	void upd(Playground &playground);

private:

	ActionImage actionImageOfAgent1, actionImageOfAgent2;
	void draw();
	void updActionImageView(Playground &playground);

};
}

