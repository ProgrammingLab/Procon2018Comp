#include "Visualizer.h"
#include "TrainDataVisualizer.h"


namespace Procon2018 {


void TrainDataVisualize() {
	/*s3d::Font font(20);
	s3d::TextBox tb(font, s3d::Vec2(), 800, s3d::none, U"train data dir");

	while (s3d::System::Update()) {
		const auto result = tb.update();
		if (result == s3d::TextBox::State::Enter) break;
		tb.draw();
		tb.drawOverlay();
	}*/

	std::string path;
	std::cout << "input game dir... >> ";
	std::cin >> path;
	s3d::RectF v(0, 0, s3d::Window::Size());
	TrainDataVisualizer tdv(v, path);
	while (s3d::System::Update()) {
		tdv.update();
	}
}


}


void Main()
{
	using namespace Procon2018;

	s3d::Graphics2D::SetSamplerState(s3d::SamplerState::ClampLinear);
	s3d::Console.open();

	TrainDataVisualize();
}
