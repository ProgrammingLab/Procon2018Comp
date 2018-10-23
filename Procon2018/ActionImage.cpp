#include "stdafx.h"
#include "ActionImage.h"
#include <Siv3D.hpp>


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
		//printf("%d %d\n", cx, cy);
	}

	void ActionImage::updActionImage(Direction8 dir) {
		actionImageId = dir;
	}

}
