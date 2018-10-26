#include "Visualizer.h"
#include "TrainDataVisualizer.h"
#include "Shared/DnnClient.h"
#include "Shared/Mcts.h"
#include "Playground.h"
#include "WinjAI/WinjAI.h"
#include "QRReader.h"
#include "ActionImageView.h"

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
	//s3d::Graphics::SetFullScreen(true, s3d::Size(1280, 720));
	SP<AI> winjAI3((AI*)new WinjAI::WinjAI3());
	SP<AI> winjAI2((AI*)new WinjAI::WinjAI2());
	Playground grd(s3d::RectF(0, 0, s3d::Window::Size().x * 2 / 3, s3d::Window::Size().y), winjAI3, nullptr);
	grd.setHiddenAI(true);
	ActionImageView actView;
	while (s3d::System::Update()) {
		grd.update();
		actView.upd(grd);
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
		if (s3d::KeyE.down()) {
			break;
		}
		fv.update();
	}
	return _field;
}

void GachiMain() {
	SP<AI> winjAI3((AI*)new WinjAI::WinjAI3());
	Field field = rotField();
	Playground grd(s3d::RectF(0, 0, s3d::Window::Size().x * 2 / 3, s3d::Window::Size().y), winjAI3, nullptr, field);
	grd.setHiddenAI(true);
	ActionImageView actView;
	while (s3d::System::Update()) {
		grd.update();
		actView.upd(grd);
	}
}


}


void Main()
{
	using namespace Procon2018;

	const s3d::Font font(30);
	s3d::Graphics2D::SetSamplerState(s3d::SamplerState::ClampLinear);
	s3d::Console.open();
	std::string s;
	std::cout << "input rand seed...(-1 for using the time) >> ";
	std::cin >> s;
	if (s == "-1") {
		Rand::InitializeWithTime();
	}
	else {
		Rand::Initialize(std::stoul(s));
	}

	//TrainDataVisualize();
	//BattleToDnn();
	//HumanPlay();
	//BattleToWinjAI();
	GachiMain();
}
