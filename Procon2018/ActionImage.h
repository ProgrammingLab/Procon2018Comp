#pragma once

#include <Siv3D.hpp>
#include "Shared/Field.h"
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

	void initTexture();

	static std::optional<s3d::Array<s3d::Texture>> dirImage;

	static std::optional<s3d::Array<s3d::Array<s3d::Texture>>> actionImages;

};
}

