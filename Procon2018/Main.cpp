#include "FieldView.h"

void Main()
{
	using namespace Procon2018;

	s3d::Graphics2D::SetSamplerState(s3d::SamplerState::ClampLinear);

	FieldView fv(s3d::Window::ClientRect());

	while (s3d::System::Update()) {
		fv.update();
	}
}
