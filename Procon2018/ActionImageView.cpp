#include "stdafx.h"
#include "ActionImageView.h"


namespace Procon2018 {

ActionImageView::ActionImageView(PlayerId pId) : pId(pId) {
	actionImageOfAgent1.setAgentIdOnPlayer(0);
	actionImageOfAgent2.setAgentIdOnPlayer(1);
	actionImageOfAgent1.cx = s3d::Window::Size().x * 2 / 3 , actionImageOfAgent1.cy = 50;
	actionImageOfAgent2.cx = s3d::Window::Size().x * 2 / 3 + actionImageOfAgent2.width, actionImageOfAgent2.cy = 50;
}


ActionImageView::~ActionImageView() {
}

void ActionImageView::upd(const std::array<OptAction, 4> &actions) {
	updActionImageView(actions);
	draw();
}

void ActionImageView::draw() {
	actionImageOfAgent1.draw();
	actionImageOfAgent2.draw();
}

void ActionImageView::updActionImageView(const std::array<OptAction, 4> &actions) {
	OptAction act1 = actions[(int)pId*2 + 0];
	OptAction act2 = actions[(int)pId*2 + 1];

	//printf("%d %d\n", act1->dir, act2->dir);



	actionImageOfAgent1.updActionImage(act1);
	actionImageOfAgent2.updActionImage(act2);

}

}
