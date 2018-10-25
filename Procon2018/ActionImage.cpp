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
		actionImages[actionImageId].resized(width, height).draw(cx, cy);
		dirImage[actionImageId].resized(width, height).draw(cx, cy + height);
	}

	void ActionImage::updActionImage(OptAction act) {
		if (!act)actionImageId = actionImages.size() - 1;
		else actionImageId = act->dir;
	}

}
