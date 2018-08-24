#include "Visualizer.h"

void Main()
{
	using namespace Procon2018;

	s3d::Graphics2D::SetSamplerState(s3d::SamplerState::ClampLinear);
	s3d::Console.open();

	Visualizer visualizer;

	while (s3d::System::Update()) {
		visualizer.update();
	}
}
