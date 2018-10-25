#pragma once

#include <Siv3D.hpp>
#include "Field.h"
#include "Shared/Util.h"


namespace Procon2018 {
class ActionImage {
public:

	ActionImage();
	ActionImage(Point pos);
	~ActionImage();


	s3d::Texture actionImage;
	const int width = s3d::Window::Size().x / 6, height = s3d::Window::Size().y / 3;

	int cx, cy;

	void updActionImage(OptAction act);
	void draw();

private:

	int actionImageId;

	s3d::Array<s3d::Texture> dirImage = { s3d::Texture(U"dir0.png"),
										  s3d::Texture(U"dir1.png"),
										  s3d::Texture(U"dir2.png"),
										  s3d::Texture(U"dir3.png"),
									      s3d::Texture(U"dir4.png"),
									      s3d::Texture(U"dir5.png"),
										  s3d::Texture(U"dir6.png"),
										  s3d::Texture(U"dir7.png"), 
									      s3d::Texture(U"") };
	s3d::Array<s3d::Texture> actionImages = { s3d::Texture(U"move0.jpg"),
											  s3d::Texture(U"move1.jpg"),
											  s3d::Texture(U"move2.jpg"),
											  s3d::Texture(U"move3.jpg"),
											  s3d::Texture(U"move4.jpg"),
											  s3d::Texture(U"move5.jpg"),
											  s3d::Texture(U"move6.jpg"),
											  s3d::Texture(U"move7.jpg"),
											  s3d::Texture(U"") };

};
}

