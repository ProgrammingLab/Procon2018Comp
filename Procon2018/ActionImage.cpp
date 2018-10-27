#include "stdafx.h"
#include "ActionImage.h"


namespace Procon2018 {

	std::optional<s3d::Array<s3d::Texture>> ActionImage::dirImage;

	std::optional<s3d::Array<s3d::Array<s3d::Texture>>> ActionImage::actionImages;

	ActionImage::ActionImage() {
		t = 0;
		initTexture();
	}

	ActionImage::ActionImage(Point pos) {
		t = 0;
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
		t += sw.sF();
		sw.restart();
		if (t >= 0.5) t -= 0.5;
		double w = 2*s3d::Math::Pi*t/0.5;
		int k = (aType == ActionType::Remove ? 0 : 0);
		int cx_ = cx + k*s3d::Math::Cos(w), cy_ = cy + k*s3d::Math::Sin(w);
		(*actionImages)[agentIdOnPlayer][actionImageId].resized(width, height).draw(cx_, cy_);
		(*dirImage)[actionImageId].resized(width, height).draw(cx_, cy_ + height);
	}

	void ActionImage::updActionImage(OptAction act) {
		if (!act) {
			actionImageId = (*actionImages)[agentIdOnPlayer].size() - 1;
		}
		else {
			actionImageId = act->dir;
			aType = act->type;
		}
	}

}
