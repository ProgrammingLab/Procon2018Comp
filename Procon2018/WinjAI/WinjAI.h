#pragma once
#include "../AI.h"
#include "../FieldView.h"
#include "Siv3D.hpp"

namespace Procon2018::WinjAI {


class WinjAI : AI {
private:

	Field m_field;

	PlayerId m_pId;

	int m_all;

	PlayerMove m_myMove;

	struct SearchData {
		bool used[Field::MAX_H][Field::MAX_W];
		std::vector<Point> path;
		double v;
		SearchData() {
			for (int i = 0; i < Field::MAX_H; i++) {
				for (int j = 0; j < Field::MAX_W; j++) {
					used[i][j] = false;
				}
			}
			v = 0;
		}
	};

public:

	WinjAI() {}

	std::vector<Point> search(
		const std::vector<SP<SearchData>> &firsts,
		std::function<double(const SearchData&)> evaluate, //同じ深さの盤面同士を比較できればいい
		std::function<double(const SearchData& current, Point np)> finalScore,
		std::function<bool(const SearchData& current, Point np)> isFinal,
		int beamWidth,
		bool output = false) {
		auto similar = [&](const SearchData &a, const SearchData &b) {
			int cnt = 0;
			for (int i = 0; i < m_field.h(); i++) {
				for (int j = 0; j < m_field.w(); j++) {
					cnt += (a.used[i][j] != b.used[i][j]);
				}
			}
			//return cnt < std::min(5, (int)std::min(a.path.size(), b.path.size())/3);
			return cnt == 0;
		};
		auto comp = [&](const SP<SearchData> a, const SP<SearchData> b) {
			return a->v > b->v;
		};

		std::vector<Point> ret;
		double max = -1e10;
		auto updateBest = [&](const SearchData &previous, Point np) {
			double v = finalScore(previous, np);
			if (max < v) {
				max = v;
				ret = previous.path;
				ret.push_back(np);
			}
		};

		using Vec = std::vector<SP<SearchData>>;
		Vec states[Field::MAX_H][Field::MAX_W][2];
		/*for (int y = 0; y < field.h(); y++) {
		for (int x = 0; x < field.w(); x++) {
		SP<SearchData> first(new SearchData());
		first->path.push_back(Point(x, y));
		first->used[y][x] = true;
		states[y][x][0].push_back(first);	
		}
		}*/
		for (int i = 0; i < firsts.size(); i++) {
			states[firsts[i]->path.back().y][firsts[i]->path.back().x][0].push_back(firsts[i]);	
		}
		int steps = m_field.h()*m_field.w()/2;
		for (int step = 0; step < steps; step++) {
			if (output) std::cout << "step:" << step << std::endl;
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					int cnt = 0;
					for (int vi = 0; cnt < beamWidth && vi < (int)states[y][x][step&1].size(); vi++) {
						SP<SearchData> t = states[y][x][step&1][vi];
						if ([&]() {
							for (int vj = 0; vj < vi; vj++) {
								if (similar(*t, *states[y][x][step&1][vj]))
									return true;
							}
							return false;
						}() ) continue;
						cnt++;

						for (int dir = 0; dir < 8; dir++) {
							Point np = Point(x, y) + Neighbour8((Direction8)dir);
							if (np.x < 0 || m_field.w() <= np.x || np.y < 0 || m_field.h() <= np.y)
								continue;
							//if (m_field.grid(np).score < 0) continue;
							/*if (np == t->path.front()) {
							updateBest(*t, np);
							}*/
							if(isFinal(*t, np)) {
								updateBest(*t, np);
								continue;
							}
							SP<SearchData> next(new SearchData(*t));
							next->v = evaluate(*next);
							next->path.push_back(np);
							next->used[np.y][np.x] = true;
							/*if ((dir&1) == 0) {
							Point cross0 = Point(x, y) + Neighbour8((Direction8)((dir + 1) % 8));
							Point cross1 = Point(x, y) + Neighbour8((Direction8)((dir + 7) % 8));
							next->used[cross0.y][cross0.x] = true;
							next->used[cross1.y][cross1.x] = true;
							}*/
							states[np.y][np.x][(step + 1)&1].push_back(next);
						}
					}
					states[y][x][step&1].clear();
				}
			}
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					auto &v = states[y][x][(step + 1)&1];
					std::sort(v.begin(), v.end(), comp);
				}
			}
		}
		return ret;
	}

	std::vector<Point> findLoop(Point start) {
		auto inner = [&](SP<SearchData> first, Point target) {
			auto evaluate = [&](const SearchData &d) {
				double v = 0;
				for (int i = 0; i < d.path.size(); i++) {
					v += m_field.grid(d.path[i]).score;
				}
				return (double)v/d.path.size();
			};
			auto finalScore = [&](const SearchData &current, Point np) {
				return evaluate(current);
			};
			auto isFinal = [&](const SearchData &current, Point np) {
				return np == target;
			};
			return search({first}, evaluate, finalScore, isFinal, 1);
		};

		auto evaluate = [&](const SearchData &d) {
			double sum = 0;
			double penalty = 0;
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					if (d.used[y][x]) {
						sum += m_field.grid(Point(x, y)).score;
						continue;
					}
					int cnt = 0;
					for (int k = 0; k < 8; k++) {
						Point q = Point(x, y) + Neighbour8((Direction8)k);
						if (d.used[q.y][q.x]) cnt++;
					}
					penalty += m_field.grid(Point(x, y)).score*(double)cnt/4.0;
				}
			}
			for (int dir = 0; dir < 8; dir++) {
				Point np = d.path.back() + Neighbour8((Direction8)dir);
				penalty += 1.0*(d.used[np.y][np.x]);
			}
			SP<SearchData> d_(new SearchData(d));
			d_->path.clear();
			d_->path.push_back(d.path.back());
			std::vector<Point> path_ = inner(d_, d.path.front());
			double sum2 = 0;
			for (int i = 0; i < path_.size(); i++) {
				sum2 += m_field.grid(path_[i]).score;
			}
			if ((double)sum2/path_.size() < 0.8*(double)sum/d.path.size()) return 1e-10;
			return sum - penalty;
		};
		auto finalScore = [&](const SearchData &current, Point np) {
			int i;
			int sum1 = 0;
			for (i = 0; i < current.path.size(); i++) {
				Point p = current.path[i];
				if (p == np) break;
				sum1 += m_field.grid(p).score;
			}
			int s1 = i;
			int sum2 = 0;
			for (; i < current.path.size(); i++) {
				Point p = current.path[i];
				sum2 += m_field.grid(p).score;
			}
			int s2 = (int)current.path.size() - s1;
			//if (sum2 < m_all/2) return -1e10;
			//if (s1 > 0)
			//	v += std::min((double)sum2, 0.5*all)*((double)sum2/current.path.size()) + 0.05*(double)sum1/s1;
			return (double)sum2/s2 + 0.1*(double)sum1/s1;
		};
		auto isFinal = [&](const SearchData &current, Point np) {
			return current.used[np.y][np.x];
		};

		SP<SearchData> first(new SearchData());
		first->path.push_back(start);
		first->used[start.y][start.x] = true;
		return search({first}, evaluate, finalScore, isFinal, 10, true);
	}

	std::pair<std::vector<Point>, std::vector<Point>> calcPath(PlayerId pId) {
		auto distance = [](Point a, Point b) {
			return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
		};

		PlayerId pId_ = (PlayerId)(1 - (int)pId);
		AgentId a0 = (AgentId)(2*(int)pId + 0);
		AgentId a1 = (AgentId)(2*(int)pId + 1);
		AgentId a0_ = (AgentId)(2*(int)pId_ + 0);
		AgentId a1_ = (AgentId)(2*(int)pId_ + 1);
		Point p0 = m_field.agentPos(a0);
		Point p1 = m_field.agentPos(a1);
		Point p0_ = m_field.agentPos(a0_);
		Point p1_ = m_field.agentPos(a1_);
		//bool kicked[Field::MAX_H][Field::MAX_W];
		auto future = [&](const bool used_[Field::MAX_H][Field::MAX_W], const Point start, const Point trg) {
			struct Data {
				Point p;
				int min;
				int cnt;
				bool operator < (const Data &d) const {
					if (min == d.min) return cnt > d.cnt;
					return min < d.min;
				}
			};
			bool used[Field::MAX_H][Field::MAX_W] = {};
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					auto g = m_field.grid(Point(x, y));
					if (used_[y][x]) used[y][x] = true;
					if (g.color && *g.color == pId) used[y][x] = true;
				}
			}
			std::priority_queue<Data> pq;
			Data first;
			first.p = start;
			first.min = (int)1e10;
			first.cnt = 0;
			pq.push(first);
			while (!pq.empty()) {
				Data t = pq.top();
				pq.pop();
				if (t.p == trg) {
					return t.min;
				}
				if (used[t.p.y][t.p.x]) continue;
				used[t.p.y][t.p.x] = true;
				for (int dir = 0; dir < 8; dir++) {
					Point np = t.p + Neighbour8((Direction8)dir);
					if (m_field.outOfField(np)) continue;
					if (used[np.y][np.x]) continue;
					auto nextGrid = m_field.grid(np);
					Data next;
					next.p = np;
					next.cnt = t.cnt;
					next.min = std::min(t.min, nextGrid.score);
					if (np == trg) {
						next.min = t.min;
					}
					pq.push(next);
					if (next.min < t.min && t.cnt < 1 && nextGrid.score >= 0) {
						next.cnt++;
						next.min = t.min;
						pq.push(next);
					}
				}
			}
			return -1;
		};
		auto evaluatePath = [&](const SearchData &d) {
			double avr = 0;
			double penalty = 0;
			int removeCost = 0;
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					auto g = m_field.grid(Point(x, y));
					if (g.color && *g.color == pId) continue;
					if (d.used[y][x]) {
						avr += g.score;
						if (g.color && *g.color == pId_) {
							avr += g.score;
							removeCost++;
						}
						continue;
					}
					int cnt = 0;
					for (int k = 0; k < 8; k++) {
						Point q = Point(x, y) + Neighbour8((Direction8)k);
						if (d.used[q.y][q.x]) cnt++;
					}
					penalty += m_field.grid(Point(x, y)).score*(double)cnt/4.0;
				}
			}
			avr /= d.path.size() + removeCost;
			penalty /= m_field.h()*m_field.w();
			return std::make_pair(avr, penalty);
		};
		auto evaluate0 = [&](const SearchData &d) {
			int min1 = future(d.used, d.path.back(), p0_);
			int min2 = future(d.used, p1, p1_);
			auto [avr, penalty] = evaluatePath(d);
			return (avr + min1 + min2)/3.0 - penalty;
		};
		auto finalScore0 = [&](const SearchData &current, Point np) {
			auto [avr, penalty] = evaluatePath(current);
			int min = future(current.used, p1, p1_);
			if (current.path.size() < distance(current.path.front(), np) + 3)
				return 1e-10;
			return avr*0.6 + min*0.4;
		};
		auto isFinal0 = [&](const SearchData &current, Point np) {
			return np == p0_;
		};
		auto evaluate1 = [&](const SearchData &d) {
			int min = future(d.used, d.path.back(), p1_);
			auto [avr, penalty] = evaluatePath(d);
			return (avr + min)/2.0 - penalty;
		};
		auto finalScore1 = [&](const SearchData &current, Point np) {
			auto [avr, penalty] = evaluatePath(current);
			if (current.path.size() < distance(current.path.front(), np) + 3)
				return 1e-10;
			return avr;
		};
		auto isFinal1 = [&](const SearchData &current, Point np) {
			return np == p1_;
		};
		std::pair<std::vector<Point>, std::vector<Point>> ret;
		{
			SP<SearchData> first(new SearchData());
			first->path.push_back(p0);
			first->used[p0.y][p0.x] = true;
			ret.first = search(
				{first},
				evaluate0,
				finalScore0,
				isFinal0,
				1,
				false);
		}
		{
			SP<SearchData> first(new SearchData());
			first->path.push_back(p1);
			first->used[p1.y][p1.x] = true;
			for (int i = 0; i < ret.first.size(); i++) {
				first->used[ret.first[i].y][ret.first[i].x] = true;
			}
			ret.second = search(
				{first},
				evaluate1,
				finalScore1,
				isFinal1,
				1,
				false);
		}
		return ret;
	}

	//TODO: 逆から探索すると強いのでは
	std::pair<std::vector<Point>, std::vector<Point>> calcFinalPath(PlayerId pId, std::optional<std::pair<std::vector<Point>, std::vector<Point>>> oppPath) {
		PlayerId pId_ = (PlayerId)(1 - (int)pId);
		AgentId a0 = (AgentId)(2*(int)pId + 0);
		AgentId a1 = (AgentId)(2*(int)pId + 1);
		AgentId a0_ = (AgentId)(2*(int)pId_ + 0);
		AgentId a1_ = (AgentId)(2*(int)pId_ + 1);
		Point p0 = m_field.agentPos(a0);
		Point p1 = m_field.agentPos(a1);
		Point p0_ = m_field.agentPos(a0_);
		Point p1_ = m_field.agentPos(a1_);

		auto evaluate = [&](const SearchData &d) {
			Field f_(m_field);
			for (int i = 0; i < d.path.size(); i++) {
				f_.setColor(d.path[i], pId);
			}
			if (oppPath) {
				for (int i = 0; i < oppPath->first.size(); i++) {
					f_.setColor(oppPath->first[i], pId_);
				}
				for (int i = 0; i < oppPath->second.size(); i++) {
					f_.setColor(oppPath->second[i], pId_);
				}
			}
			int removeCost = 0;
			for (int i = 0; i < d.path.size(); i++) {
				auto g = f_.grid(d.path[i]);
				if (g.color && *g.color == pId_) removeCost++;
			}
			if (m_field.resTurn() - removeCost - d.path.size() + 1 < 0 && f_.grid(d.path.back()).color && f_.grid(d.path.back()).color == pId)
				f_.setColor(d.path.back(), std::optional<PlayerId>());
			auto res = f_.calcScore();
			if (pId == PlayerId::A) return res.first - res.second;
			return res.second - res.first;
		};
		auto finalScore = [&](const SearchData &current, Point np) {
			SearchData current_(current);
			current_.path.push_back(np);
			return evaluate(current_);
		};
		auto isFinal = [&](const SearchData &current, Point np) {
			Field f_(m_field);
			std::vector<Point> myPath = current.path;
			myPath.push_back(np);
			for (int i = 0; i < myPath.size(); i++) {
				f_.setColor(myPath[i], pId);
			}
			f_.setColor(np, pId);
			if (oppPath) {
				for (int i = 0; i < oppPath->first.size(); i++) {
					f_.setColor(oppPath->first[i], pId_);
				}
				for (int i = 0; i < oppPath->second.size(); i++) {
					f_.setColor(oppPath->second[i], pId_);
				}
			}
			int removeCost = 0;
			for (int i = 0; i < myPath.size(); i++) {
				auto g = f_.grid(myPath[i]);
				if (g.color && *g.color == pId_) removeCost++;
			}
			return m_field.resTurn() - removeCost - current.path.size() <= 0;
		};
		std::pair<std::vector<Point>, std::vector<Point>> ret;
		{
			SP<SearchData> first(new SearchData());
			first->path.push_back(p0);
			first->used[p0.y][p0.x] = true;
			ret.first = search(
				{first},
				evaluate,
				finalScore,
				isFinal,
				1,
				false);
		}
		{
			SP<SearchData> first(new SearchData());
			first->path.push_back(p1);
			first->used[p1.y][p1.x] = true;
			for (int i = 0; i < ret.first.size(); i++) {
				first->used[ret.first[i].y][ret.first[i].x] = true;
			}
			ret.first = search(
				{first},
				evaluate,
				finalScore,
				isFinal,
				1,
				false);
		}
		return ret;
	}

	virtual void init(const Field & field, PlayerId playerId) override {
		m_field = field;
		m_pId = playerId;
		m_all = 0;
		for (int y = 0; y < m_field.h(); y++) {
			for (int x = 0; x < m_field.w(); x++) {
				int s = m_field.grid(Point(x, y)).score;
				if (s > 0) m_all += s;
			}
		}
	}

	virtual PlayerMove calcNextMove(std::optional<std::pair<PlayerMove, PlayerMove>> moves) override {
		auto finalProc = [&](PlayerId pId) {
			PlayerId pId_ = (PlayerId)(1 - (int)m_pId);

			std::optional<std::pair<std::vector<Point>, std::vector<Point>>> prev;
			for (int i = 0; i < 10; i++) {
				prev = calcFinalPath(pId_, prev);
				prev = calcFinalPath(pId, prev);
			}
			return *prev;
		};

		if (moves) {
			m_field.forward(moves->first, moves->second);
		}
		auto res = (m_field.resTurn() < 0) ? finalProc(m_pId) : calcPath(m_pId);
		std::vector<Point> paths[2] = { res.first, res.second };
		OptAction actions[2];
		for (int i = 0; i < 2; i++) {
			int dir = -1;
			Point np;
			for (dir = 0; dir < 8; dir++) {
				np = paths[i].front() + Neighbour8((Direction8)dir);
				if (np == paths[i][1]) break;
			}
			if (dir == -1) throw "oh...";
			ActionType aType = ActionType::Move;
			auto g = m_field.grid(np);
			if (g.color && *g.color != m_pId) aType = ActionType::Remove;
			actions[i] = Action(aType, (Direction8)dir);
		}
		return PlayerMove(actions[0], actions[1]);
	}
};


void Tmp() {
	Field field = Field::RandomState();
	WinjAI ai;
	ai.init(field, PlayerId::A);
	auto res = ai.calcPath((PlayerId)0);
	std::vector<Point> paths[2] = {res.first, res.second};
	FieldView fv(s3d::RectF(0, 0, s3d::Window::Size()), field);
	s3d::Stopwatch sw;
	sw.start();
	int i = 0, j = 0;
	while (s3d::System::Update()) {
		if (sw.sF() > 0.5) {
			sw.restart();
			field.setPos((AgentId)i, paths[i][j]);
			field.setColor(paths[i][j], std::make_optional(PlayerId::A));
			fv.transit(field);
			j++;
			if (j >= paths[i].size()) {
				j = 0;
				i = 1 - i;
				if (i == 0) {
					for (int y = 0; y < field.h(); y++) {
						for (int x = 0; x < field.w(); x++) {
							field.setColor(Point(x, y), std::optional<PlayerId>());
						}
					}
				}
			}
		}
		fv.update();
	}
}


}
