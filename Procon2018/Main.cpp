#include "Visualizer.h"

void Main()
{
	using namespace Procon2018;

	s3d::Graphics2D::SetSamplerState(s3d::SamplerState::ClampLinear);

	Visualizer visualizer;

	while (s3d::System::Update()) {
		visualizer.update();
	}
}
