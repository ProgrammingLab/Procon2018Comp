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

	void setAgentId(AgentId id) { agentId = id; }

private:

	int actionImageId;
	
	AgentId agentId;


	s3d::Array<s3d::Texture> dirImage = { s3d::Texture(U"dir0.png"),
										  s3d::Texture(U"dir1.png"),
										  s3d::Texture(U"dir2.png"),
										  s3d::Texture(U"dir3.png"),
									      s3d::Texture(U"dir4.png"),
									      s3d::Texture(U"dir5.png"),
										  s3d::Texture(U"dir6.png"),
										  s3d::Texture(U"dir7.png"), 
									      s3d::Texture(U"") };

	s3d::Array<s3d::Array<s3d::Texture>> actionImages = { 
											{ s3d::Texture(U"move0_left.jpg"),
											  s3d::Texture(U"move1_left.jpg"),
											  s3d::Texture(U"move2_left.jpg"),
											  s3d::Texture(U"move3_left.jpg"),
											  s3d::Texture(U"move4_left.jpg"),
											  s3d::Texture(U"move5_left.jpg"),
											  s3d::Texture(U"move6_left.jpg"),
											  s3d::Texture(U"move7_left.jpg"),
											  s3d::Texture(U"aaaa") 
											},

											{ s3d::Texture(U"move0_right.jpg"),
											  s3d::Texture(U"move1_right.jpg"),
											  s3d::Texture(U"move2_right.jpg"),
											  s3d::Texture(U"move3_right.jpg"),
											  s3d::Texture(U"move4_right.jpg"),
											  s3d::Texture(U"move5_right.jpg"),
											  s3d::Texture(U"move6_right.jpg"),
											  s3d::Texture(U"move7_right.jpg"),
											  s3d::Texture(U"aaaa")
											},
									
											
											};

};
}

