#include "stdafx.h"
#include "ActionImageView.h"


namespace Procon2018 {

ActionImageView::ActionImageView() {
	actionImageOfAgent1.cx = s3d::Window::Size().x * 2 / 3 , actionImageOfAgent1.cy = 50;
	actionImageOfAgent2.cx = s3d::Window::Size().x * 2 / 3 + actionImageOfAgent2.width, actionImageOfAgent2.cy = 50;
}


ActionImageView::~ActionImageView() {
}

void ActionImageView::upd(Playground &playground) {
	updActionImageView(playground);
	draw();
}

void ActionImageView::draw() {
	actionImageOfAgent1.draw();
	actionImageOfAgent2.draw();
}

void ActionImageView::updActionImageView(Playground &playground) {
	OptAction act1 = playground.getActions()[0];
	OptAction act2 = playground.getActions()[1];

	printf("%d %d\n", act1->dir, act2->dir);



	actionImageOfAgent1.updActionImage(act1->dir);
	actionImageOfAgent2.updActionImage(act2->dir);

}

}
