﻿#include "../Procon2018/Shared/Util.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <direct.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/asio.hpp>
#include <omp.h>

#include "../Procon2018/Shared/Field.h"
#include "../Procon2018/Shared/DnnClient.h"
#include "../Procon2018/Shared/Mcts.h"

using namespace Procon2018;


std::string ToStr(double d) {
	std::stringstream ss;
	ss << std::setprecision(16) << d;
	return ss.str();
}


bool CheckFolder(const std::string folderName) {
	if( _mkdir( folderName.c_str() ) == 0 ){
		return true;
	} else {
		return false;
	}
}

void SelfPlay(int gameCount, std::string outputIp, unsigned short outputPort) {
	SP<DnnClient> dnn(new DnnClient("127.0.0.1", 54215));
	std::vector<Mcts> trees;
	for (int i = 0; i < gameCount; i++) {
		trees.push_back(Mcts(Field::RandomState(), dnn));
	}

	struct Log {
		Field field;
		double q;
		VisitCount visitCount;
		Log(const Field &field, double q, const VisitCount &visitCount)
			: field(field)
			, q(q)
			, visitCount(visitCount) {}
	};
	std::vector<std::vector<Log>> logs(gameCount);

	int globalTurn = 0;
	while (true) {
		std::cout << "turn: " << globalTurn << std::endl;
		std::vector<std::pair<int, Mcts&>> tp;
		for (int i = 0; i < gameCount; i++) {
			if (!trees[i].isEnd()) tp.push_back(std::pair<int, Mcts&>(i, trees[i]));
		}
		int n = (int)tp.size();
		if (n == 0) break;

		std::vector<std::vector<IntMoves>> paths(n);
		std::vector<bool> expands(n);
		std::vector<Field> states(n);

		std::vector<PolicyPair> policies(n);
		std::vector<double> values;
		for (int simCount = 0; simCount < 1000; simCount++) {
			std::cout << ".";
			if (simCount%100 == 99) std::cout << std::endl;
			paths.clear();
			paths.resize(n);
#pragma omp parallel
			{
#pragma omp for
				for (int i = 0; i < n; i++) {
					states[i] = tp[i].second.copyRootState();
					expands[i] = !tp[i].second.goDown(states[i], paths[i]);
				}
#pragma omp single
				{
					values = dnn->Evaluate(states, policies);
				}
#pragma omp for
				for (int i = 0; i < n; i++) {
					tp[i].second.backup(paths[i], values[i], policies[i], expands[i]);
				}
			}
		}

		for (int i = 0; i < n; i++) {
			double q = 0;
			SP<Node> node = tp[i].second.root();
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < PlayerMove::IntCount(); k++) {
					q += node->m_w[j][k];
				}
				q /= tp[i].second.root()->m_countSum;
			}
			logs[tp[i].first].push_back(Log(tp[i].second.copyRootState(), q, node->m_count));

			tp[i].second.selfNext(0, dnn);
		}
		globalTurn++;
	}

	using namespace boost::property_tree;
	for (int gameId = 0; gameId < gameCount; gameId++) {
		Field state = trees[gameId].copyRootState();
		double z = 0.0;
		auto gameRes = state.calcScore();
		if (gameRes.first > gameRes.second) z = +1.0;
		if (gameRes.first < gameRes.second) z = -1.0;

		ptree pt;
		ptree data;
		for (int turn = 0; turn < (int)logs[gameId].size(); turn++) {
			ptree childData;
			childData.add_child("state", state.toPTree());
			childData.put("q", ToStr(logs[gameId][turn].q));
			childData.put("z", ToStr(z));
			{
				ptree visitCount;
				for (int i = 0; i < 2; i++) {
					ptree child;
					for (int j = 0; j < PlayerMove::IntCount(); j++) {
						ptree unit;
						unit.put("", ToStr(logs[gameId][turn].visitCount[i][j]));
						child.push_back(std::make_pair("", unit));
					}
					visitCount.push_back(std::make_pair("", child));
				}
				childData.add_child("visitCount", visitCount);
			}
			data.push_back(std::make_pair("", childData));
		}
		pt.add_child("data", data);

		namespace asio = boost::asio;
		using asio::ip::tcp;

		asio::io_service ios()
	}
}

void MctsTest() {
	int score[4][5] = {
		{0, 0, 1, 0, 0},
	{0,-2, 1,-2, 0},
	{0, 0, 1, 0, 0},
	{0, 0, 1, 0, 0}
	};
	int color[4][5] = {
		{1, 1, 0, 0, 1},
	{0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0},
	{2, 0, 0, 0, 2}
	};
	std::vector<std::vector<Grid>> fld(4);
	for (int i = 0; i < 4; i++) {
		fld[i].resize(5);
		for (int j = 0; j < 5; j++) {
			fld[i][j].score = score[i][j];
			if (color[i][j] == 1) fld[i][j].color = std::make_optional(PlayerId::A);
			if (color[i][j] == 2) fld[i][j].color = std::make_optional(PlayerId::B);
		}
	}
	std::array<Point, 4> agent = {Point(0, 0), Point(4, 0), Point(0, 3), Point(4, 3)};
	Field first(2, 4, 5, fld, agent);
	SP<DnnClient> dnn = SP<DnnClient>(new DnnClient("127.0.0.1", 54215));
	Mcts mcts(first, dnn);
	for (int i = 0; i < 1000; i++) {
		std::cout << i << std::endl;
		Field state = mcts.copyRootState();
		std::vector<IntMoves> path;
		bool expands = !mcts.goDown(state, path);

		PolicyPair policyPair;
		double v = dnn->Evaluate(state, policyPair);

		mcts.backup(path, v, policyPair, expands);
	}

	// debug output
	/*SP<Node> root = mcts.root();
	for (IntMove i = 0; i < PlayerMove::IntCount(); i++) {
	auto show = [](const OptAction &a) {
	if (!a) return std::string("..");
	std::string ret = (a.value().type == ActionType::Move ? "m" : "r");
	ret += std::to_string((int)a.value().dir);
	return ret;
	};
	int p0 = root->m_count[0][(int)i];
	int p1 = root->m_count[1][(int)i];
	PlayerMove move = PlayerMove::FromInt(i);
	std::cout << "(" + show(move.a0) + "," << show(move.a1) << ") : (" << p0 << "," << p1 << ")" << std::endl;
	}*/

	mcts.selfNext(0, dnn);
}

int main(int argc, char* argv[])
{
	std::stringstream ss(argv[1]);
	int gameCount;
	ss >> gameCount;
	CheckFolder(argv[2]);
	SelfPlay(gameCount, argv[2]);

	/*
	Field field = Field::RandomState();
	SP<DnnClient> dnn;
	while (true) {
	try {
	dnn = SP<DnnClient>(new DnnClient("127.0.0.1", 54215));
	}
	catch (...) {
	std::cout << "dnn initialize failed" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	break;
	}
	std::cout << "initialized dnn!" << std::endl;
	PolicyPair res;
	double v = dnn->Evaluate(field, res);
	std::cout << v << std::endl;
	for (int i = 0; i < res.size(); i++) {
	for (int j = 0; j < res[i].size(); j++) {
	std::cout << res[i][j] << " ";
	}
	std::cout << std::endl;
	}
	// */


	/*
	Field field = Field::RandomState();
	using namespace boost::property_tree;
	ptree pt = field.toPTree();
	std::stringstream ss;
	write_json(ss, pt, true);
	std::cout << ss.str();
	// */

	/*
	namespace asio = boost::asio;
	using asio::ip::tcp;

	asio::io_service io_service;
	tcp::socket socket(io_service);

	boost::system::error_code error;
	socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 54215), error);

	if (error) {
	std::cout << "connect failed : " << error.message() << std::endl;
	}
	else {
	std::cout << "connected" << std::endl;
	}

	using namespace boost::property_tree;
	ptree pt;
	pt.put("Ho.Ge", "hoge");
	std::stringstream ss;
	write_json(ss, pt);
	std::string body = ss.str();
	std::string sign = std::to_string(body.size()*sizeof(char));
	if (sign.size() > 10) throw "too large";
	while (sign.size() < 10) sign.push_back(' ');
	asio::write(socket, asio::buffer(sign)); //データの長さの10バイト文字列表現
	asio::write(socket, asio::buffer(body)); //自動で全部送るらしい
	// */

	/*
	using namespace boost::property_tree;
	ptree pt;
	pt.put("Ho.Ge", u8"ほげ"); //文字化けするよ
	write_json("out.json", pt);
	// */
	return 0;
}