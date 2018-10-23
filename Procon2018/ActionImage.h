#pragma once

#include <Siv3D.hpp>
#include "Shared/Util.h"


namespace Procon2018 {
class ActionImage {
public:

	ActionImage();
	ActionImage(Point pos);
	~ActionImage();

	int actionImageId;
	const int width = s3d::Window::Size().x / 6, height = s3d::Window::Size().x / 6;

	int cx, cy;

	void updActionImage(Direction8 dir);
	void draw();

private:
	s3d::Array<s3d::Texture> actionImages = { s3d::Texture(U"dir0.png"),
											  s3d::Texture(U"dir1.png"),
											  s3d::Texture(U"dir2.png"),
											  s3d::Texture(U"dir3.png"),
											  s3d::Texture(U"dir4.png"),
											  s3d::Texture(U"dir5.png"),
											  s3d::Texture(U"dir6.png"),
											  s3d::Texture(U"dir7.png") };

};
}

