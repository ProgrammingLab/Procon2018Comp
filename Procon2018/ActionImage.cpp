#include "stdafx.h"
#include "ActionImage.h"


namespace Procon2018 {
	ActionImage::ActionImage() {
	}

	ActionImage::ActionImage(Point pos) {
		cx = pos.x;
		cy = pos.y;
	}

	ActionImage::~ActionImage() {
	}

	void ActionImage::draw() {
		actionImages[(int)agentId][actionImageId].resized(width, height).draw(cx, cy);
		dirImage[actionImageId].resized(width, height).draw(cx, cy + height);
	}

	void ActionImage::updActionImage(OptAction act) {
		if (!act)actionImageId = actionImages[(int)agentId].size() - 1;
		else actionImageId = act->dir;
	}

}
