#include "stdafx.h"
#include "Panel.h"

namespace Procon2018 {

Panel::Panel() {
}

Panel::Panel(Point pos, int width, int height, s3d::String _text) {
	cx = pos.x;
	cy = pos.y;
	w = width;
	h = height;
	text.setText(_text);
	text.setCenter(s3d::Vec2(cx, cy));
}

Panel::~Panel() {
}

void Panel::draw() {
	std::cout << "hoge" << std::endl;
	s3d::RectF(s3d::Vec2(cx - w / 2, cy - h / 2), s3d::Vec2(w, h)).draw(s3d::Palette::Greenyellow);
	text.draw();
}

bool Panel::isClicked() {

	std::optional<Point> clickedPos;
	if (s3d::MouseL.down())clickedPos = Point(s3d::Cursor::Pos().x, s3d::Cursor::Pos().y);
	if (clickedPos) {
		if (clickedPos->x < cx -w / 2 || clickedPos->y < cy - w / 2 || clickedPos->x > cx + w / 2 || clickedPos->y > cy + h / 2)return true;
		else return true;
	}
	else return false;
}



}
