#include "Visualizer.h"
#include "TrainDataVisualizer.h"
#include "Shared/DnnClient.h"
#include "Shared/Mcts.h"


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

void Battle() {
	auto toMove = [](const std::string &s) {
		if (s[0] == 'x') return OptAction();
		ActionType type = ActionType::Move;
		if (s[0] == 'r') type = ActionType::Remove;
		int dir = s[1] - '0';
		return OptAction(Action(type, (Direction8)dir));
	};

	SP<DnnClient> dnn(new DnnClient("127.0.0.1", 54215));
	Field first = Field::RandomState();
	Mcts mcts(first, dnn);
	PolicyPair policyPair;
	s3d::RectF v(0, 0, s3d::Window::Size());
	FieldView fv(v, first);
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
				mcts.backupWithExpansion(path, v, policyPair);
			}
			else
				mcts.backup(path, state.value());
		}
		puts("");

		std::cout << "input move... >> ";
		std::string s, t;
		std::cin >> s >> t;
		PlayerMove m(toMove(s), toMove(t));

		mcts.next(m, dnn);
		fv.transit(mcts.copyRootState());
	}
}


}


void Main()
{
	using namespace Procon2018;

	s3d::Graphics2D::SetSamplerState(s3d::SamplerState::ClampLinear);
	s3d::Console.open();

	//TrainDataVisualize();
	Battle();

	/*
	using namespace boost::property_tree;
	std::ifstream ifs("2.json");

	auto toStr = [](OptAction a) {
		if (!a) return std::string("  ");
		std::string s = a->type == ActionType::Move ? "m" : "r";
		return s + std::to_string(a->dir);
	};
	ptree pt;
	read_json(ifs, pt);
	Field state = Field::FromPTree(pt.get_child("state"));
	DnnClient dnn("127.0.0.1", 54215);
	PolicyPair pp;
	double v = dnn.Evaluate(state, pp);
	std::cout << "value: " << v << std::endl;
	for (int i = 0; i < 2; i++) {
		std::cout << std::endl;
		for (int j = 0; j < PlayerMove::IntCount(); j++) {
			PlayerMove m = PlayerMove::FromInt(j);
			std::cout << j << "(" << toStr(m.a0) << "," << toStr(m.a1) << "): " << pp[i][j] << std::endl;
		}
	}
	std::cout << state.value() << std::endl;
	*/
}
