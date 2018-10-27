#include "stdafx.h"
#include "ActionImage.h"


namespace Procon2018 {

	std::optional<s3d::Array<s3d::Texture>> ActionImage::dirImage;

	std::optional<s3d::Array<s3d::Array<s3d::Texture>>> ActionImage::actionImages;

	ActionImage::ActionImage() {
		initTexture();
	}

	ActionImage::ActionImage(Point pos) {
		initTexture();
		cx = pos.x;
		cy = pos.y;
	}

	ActionImage::~ActionImage() {
	}

	void ActionImage::initTexture() {
		if (!dirImage)
			dirImage = {
				s3d::Texture(U"dir0.png"),
				s3d::Texture(U"dir1.png"),
				s3d::Texture(U"dir2.png"),
				s3d::Texture(U"dir3.png"),
				s3d::Texture(U"dir4.png"),
				s3d::Texture(U"dir5.png"),
				s3d::Texture(U"dir6.png"),
				s3d::Texture(U"dir7.png"),
				s3d::Texture(U"")
			};
		if (!actionImages)
			actionImages = {
			{
				s3d::Texture(U"move0_left.jpg"),
				s3d::Texture(U"move1_left.jpg"),
				s3d::Texture(U"move2_left.jpg"),
				s3d::Texture(U"move3_left.jpg"),
				s3d::Texture(U"move4_left.jpg"),
				s3d::Texture(U"move5_left.jpg"),
				s3d::Texture(U"move6_left.jpg"),
				s3d::Texture(U"move7_left.jpg"),
				s3d::Texture(U"aaaa")
			},
			{
				s3d::Texture(U"move0_right.jpg"),
				s3d::Texture(U"move1_right.jpg"),
				s3d::Texture(U"move2_right.jpg"),
				s3d::Texture(U"move3_right.jpg"),
				s3d::Texture(U"move4_right.jpg"),
				s3d::Texture(U"move5_right.jpg"),
				s3d::Texture(U"move6_right.jpg"),
				s3d::Texture(U"move7_right.jpg"),
				s3d::Texture(U"aaaa")
			} };
	}

	void ActionImage::draw() {
		(*actionImages)[(int)agentId][actionImageId].resized(width, height).draw(cx, cy);
		(*dirImage)[actionImageId].resized(width, height).draw(cx, cy + height);
	}

	void ActionImage::updActionImage(OptAction act) {
		if (!act)actionImageId = (*actionImages)[(int)agentId].size() - 1;
		else actionImageId = act->dir;
	}

}
