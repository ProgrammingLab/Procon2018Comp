#include "Visualizer.h"
#include "TrainDataVisualizer.h"
#include "Shared/DnnClient.h"
#include "Shared/Mcts.h"
#include "Playground.h"
#include "WinjAI/WinjAI.h"
#include "QRReader.h"
#include "ActionImageView.h"
#include "PlaygroundForPad.h"

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

void BattleToDnn() {
	auto toMove = [](const std::string &s) {
		if (s[0] == 'x') return OptAction();
		ActionType type = ActionType::Move;
		if (s[0] == 'r') type = ActionType::Remove;
		int dir = s[1] - '0';
		return OptAction(Action(type, (Direction8)dir));
	};

	SP<DnnClient> dnn(new DnnClient("127.0.0.1", 54215));
	// Field first = Field::RandomState();
	using namespace boost::property_tree;
	std::string s;
	std::cin >> s;
	std::ifstream ifs(s);
	ptree pt;
	read_json(ifs, pt);
	Field first = Field::FromPTree(pt.get_child("state"));
	Mcts mcts(first, dnn);
	PolicyPair policyPair;
	s3d::RectF v(0, 0, s3d::Window::Size());
	FieldView fv(v, first);
	double q = 0;
	while (true) {
		s3d::Stopwatch sw;
		sw.start();
		while (s3d::System::Update()) {
			if (sw.sF() >= 1.0) break;
			fv.update();
		}

		auto score = mcts.copyRootState().calcScore();
		std::cout << "your score: " << score.second - score.first << std::endl;

		for (int i = 0; i < 1000; i++) {
			printf("\r%d/1000", i);
			Field state = mcts.copyRootState();
			std::vector<IntMoves> path;
			bool expands = !mcts.goDown(state, path);

			if (expands) {
				double v = dnn->Evaluate(state, policyPair);
				q += v;
				mcts.backupWithExpansion(path, v, policyPair);
			}
			else {
				double v = state.value();
				q += v;
				mcts.backup(path, v);
			}
		}
		puts("");
		q /= 1000;

		std::cout << "input move... >> ";
		std::string s, t;
		std::cin >> s >> t;
		PlayerMove m(toMove(s), toMove(t));

		mcts.next(m, dnn);
		fv.transit(mcts.copyRootState());
	}
}

void HumanPlay() {
	Playground grd(s3d::RectF(0, 0, s3d::Window::Size()));
	while (s3d::System::Update()) {
		grd.update();
	}
}

void BattleToWinjAI() {
	std::string s;
	std::cout << "input rand seed...(-1 for using the time) >> ";
	std::cin >> s;
	s = "-1";
	if (s == "-1") {
		Rand::InitializeWithTime();
	}
	else {
		Rand::Initialize(std::stoul(s));
	}
	SP<AI> winjAI3((AI*)new WinjAI::WinjAI3());
	Playground grd(s3d::RectF(0, 0, s3d::Window::Size()), winjAI3, nullptr);
	grd.setHiddenAI(true);
	while (s3d::System::Update()) {
		grd.update();
	}
}

Field rotField() {
	Field _field = QRReader().createField();
	auto createFV = [&]() {
		return FieldView(s3d::RectF(0, 0, s3d::Window::Size().x * 2 / 3, s3d::Window::Size().y), _field);
	};
	FieldView fv = createFV();
	while (s3d::System::Update()) {
		if (s3d::KeyT.down()) {
			_field.rot();
			fv = createFV();
		}
		if (s3d::KeyS.down()) {
			_field.swap();
			fv = createFV();
		}
		if (s3d::KeyE.down()) {
			break;
		}
		fv.update();
	}
	return _field;
}

void GachiMain() {
	Rand::InitializeWithTime();
	SP<AI> winjAI3((AI*)new WinjAI::WinjAI3());
	Field field = rotField();
	PlaygroundForPad grd(s3d::RectF(0, 0, s3d::Window::Size().x * 2 / 3, s3d::Window::Size().y), winjAI3, nullptr, field, PlayerId::A);
	ActionImageView actView;
	while (s3d::System::Update()) {
		grd.update();
		actView.upd(grd.getActions());
	}
}

void XInputTest() {
	using namespace s3d;
	Window::Resize(640, 480);
	Graphics::SetBackground(Color(160, 200, 100));

	const s3d::Ellipse buttonLB(160, 140, 50, 24);
	const s3d::Ellipse buttonRB(520, 140, 50, 24);

	const RectF leftTrigger(150, 16, 40, 100);
	const RectF rightTrigger(500, 16, 40, 100);

	const Circle buttonLThumb(170, 250, 35);
	const Circle buttonRThumb(420, 350, 35);
	const Circle buttonDPad(260, 350, 40);

	const Circle buttonA(510, 300, 20);
	const Circle buttonB(560, 250, 20);
	const Circle buttonX(460, 250, 20);
	const Circle buttonY(510, 200, 20);
	const Circle buttonBack(270, 250, 15);
	const Circle buttonStart(410, 250, 15);

	size_t userIndex = 0;
	bool deadZone = false;
	double leftV = 0.0, rightV = 0.0;

	while (System::Update())
	{
		auto controller = XInput(userIndex);

		if (deadZone)
		{
			controller.setLeftTriggerDeadZone();
			controller.setRightTriggerDeadZone();
			controller.setLeftThumbDeadZone();
			controller.setRightThumbDeadZone();
		}
		else
		{
			controller.setLeftTriggerDeadZone(DeadZone::Disabled());
			controller.setRightTriggerDeadZone(DeadZone::Disabled());
			controller.setLeftThumbDeadZone(DeadZone::Disabled());
			controller.setRightThumbDeadZone(DeadZone::Disabled());
		}

		controller.setVibration(leftV, rightV);

		buttonLB.draw(Color(controller.buttonLB.pressed() ? 255 : 200));
		buttonRB.draw(Color(controller.buttonRB.pressed() ? 255 : 200));

		s3d::Ellipse(340 + 3.0 * Random(leftV + rightV), 480, 300, 440).draw(Color(232));
		s3d::Ellipse(340, 40, 220, 120).draw(Color(160, 200, 100));
		Circle(340, 660, 240).draw(Color(160, 200, 100));

		Circle(340, 250, 30).draw(Color(160));

		if (controller.isConnected())
		{
			Circle(340, 250, 32).drawPie(-0.5*Math::Pi + 0.5*Math::Pi * controller.userIndex, 0.5*Math::Pi, Color(200, 255, 120));
		}

		Circle(340, 250, 25).draw(Color(140));

		leftTrigger.draw(Alpha(64));
		leftTrigger.stretched((controller.leftTrigger - 1.0) * leftTrigger.h, 0, 0, 0).draw();

		rightTrigger.draw(Alpha(64));
		rightTrigger.stretched((controller.rightTrigger - 1.0) * rightTrigger.h, 0, 0, 0).draw();

		buttonLThumb.draw(Color(controller.buttonLThumb.pressed() ? 220 : 127));
		Circle(buttonLThumb.center + Vec2(controller.leftThumbX, -controller.leftThumbY) * 25, 20).draw();

		buttonRThumb.draw(Color(controller.buttonRThumb.pressed() ? 220 : 127));
		Circle(buttonRThumb.center + Vec2(controller.rightThumbX, -controller.rightThumbY) * 25, 20).draw();

		buttonDPad.draw(Color(127));

		const Vec2 direction(
			controller.buttonRight.pressed() - controller.buttonLeft.pressed(),
			controller.buttonDown.pressed() - controller.buttonUp.pressed());

		if (!direction.isZero())
		{
			Circle(buttonDPad.center + direction.normalized() * 25, 15).draw();
		}

		buttonA.draw(Color(0, 255, 64).setA(controller.buttonA.pressed() ? 255 : 64));
		buttonB.draw(Color(255, 0, 64).setA(controller.buttonB.pressed() ? 255 : 64));
		buttonX.draw(Color(0, 64, 255).setA(controller.buttonX.pressed() ? 255 : 64));
		buttonY.draw(Color(255, 127, 0).setA(controller.buttonY.pressed() ? 255 : 64));

		buttonBack.draw(Color(controller.buttonBack.pressed() ? 255 : 200));
		buttonStart.draw(Color(controller.buttonStart.pressed() ? 255 : 200));

		SimpleGUI::RadioButtons(userIndex, { U"1P", U"2P", U"3P", U"4P" }, Vec2(10, 10));
		SimpleGUI::CheckBox(deadZone, U"DeadZone", Vec2(240, 10));
		SimpleGUI::Slider(U"低周波", leftV, Vec2(240, 50));
		SimpleGUI::Slider(U"高周波", rightV, Vec2(240, 90));
	}
}

void PlaygroundForPadTest() {
	Rand::InitializeWithTime();
	SP<AI> winjAI3((AI*)new WinjAI::WinjAI3());
	PlaygroundForPad grd(s3d::RectF(0, 0, s3d::Window::Size()), winjAI3, nullptr, PlayerId::A);
	while (s3d::System::Update()) {
		grd.update();
	}
}


}


void Main()
{
	using namespace Procon2018;

	s3d::Graphics2D::SetSamplerState(s3d::SamplerState::ClampLinear);
	s3d::Console.open();

	//TrainDataVisualize();
	//BattleToDnn();
	//HumanPlay();
	//BattleToWinjAI();
	GachiMain();
	//XInputTest();
	//PlaygroundForPadTest();
}
