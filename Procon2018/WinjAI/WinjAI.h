#pragma once
#include "../AI.h"
#include "../FieldView.h"
#include "Siv3D.hpp"
#include "omp.h"

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

	int distance(Point a, Point b) {
		return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
	};

	std::vector<Point> search(
		const std::vector<SP<SearchData>> &firsts,
		std::function<double(const SearchData&)> evaluate, //同じ深さの盤面同士を比較できればいい
		std::function<double(const SearchData& current, Point np)> finalScore,
		std::function<bool(const SearchData& current, Point np)> isFinal,
		int beamWidth,
		bool output = false,
		int steps = -1) {
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
		if (steps < 0) steps = m_field.h()*m_field.w()/2;
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
							if (t->used[np.y][np.x]) next->v -= 1000;
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
						if (m_field.outOfField(q)) continue;
						if (d.used[q.y][q.x]) cnt++;
					}
					penalty += m_field.grid(Point(x, y)).score*(double)cnt/4.0;
				}
			}
			for (int dir = 0; dir < 8; dir++) {
				Point np = d.path.back() + Neighbour8((Direction8)dir);
				if (m_field.outOfField(np)) continue;
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

	void test(PlayerId pId) {
		const int H = Field::MAX_H;
		const int W = Field::MAX_W;
		const int h = m_field.h();
		const int w = m_field.w();
		const int INT_INF = 1e5;
		auto warshallFloyd = [&](int d[H][W][H][W], const Field &fld) {
			for (int y = 0; y < H; y++) {
				for (int x = 0; x < W; x++) {
					for (int y_ = 0; y_ < H; y_++) {
						for (int x_ = 0; x_ < W; x_++) {
							d[y][x][y_][x_] = INT_INF;
						}
					}
				}
			}
			for (int y = 0; y < H; y++) {
				for (int x = 0; x < W; x++) {
					auto color = fld.grid(Point(x, y)).color;
					if (!(color && *color == pId)) continue;
					d[y][x][y][x] = 0;
					for (int dir = 0; dir < 8; dir++) {
						Point np = Point(x, y) + Neighbour8((Direction8)dir);
						if (fld.outOfField(np)) continue;
						auto c = fld.grid(np).color;
						if (c && *c == pId) d[y][x][np.y][np.x] = 1;
					}
				}
			}
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					for (int y = 0; y < h; y++) {
						for (int x = 0; x < w; x++) {
							for (int y_ = 0; y_ < h; y_++) {
								for (int x_ = 0; x_ < w; x_++) {
									int c = d[y][x][i][j] + d[i][j][y_][x_];
									d[y][x][y_][x_] = std::min(d[y][x][y_][x_], c);
								}
							}
						}
					}
				}
			}
		};
		int d[H][W][H][W];
		warshallFloyd(d, m_field);
		double max = -1e10;
		Point s, e;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				Point p(x, y);
				for (int y_ = 0; y_ < h; y_++) {
					for (int x_ = 0; x_ < w; x_++) {
						Point p_(x_, y_);
						int dist = distance(p, p_);
						if (dist <= 1 || d[y][x][y_][x_] >= INT_INF) continue;
						double v = d[y][x][y_][x_]/(double)dist;
						if (max < v) {
							max = v;
							s = p;
							e = p_;
						}
					}
				}
			}
		}
		if (max < 0) return;
		std::vector<Point> path = [&](){
			auto evaluate = [&](const SearchData &d) {
				bool u[Field::MAX_H][Field::MAX_W] = {};
				for (int i = 0; i < d.path.size(); i++) {
					u[d.path[i].y][d.path[i].x] = true;
				}
				for (int i = 0; i < d.path.size(); i++) {
					int cnt = 0;
					for (int k = 0; k < 8; k++) {
							Point np = d.path[i] + Neighbour8((Direction8)k);
							if (m_field.outOfField(np)) continue;
							if (u[np.y][np.x]) cnt++;
					}
					if (cnt >= 3) return -1e10;
				}
				double v = 0;
				for (int i = 1; i < d.path.size(); i++) {
					v += m_field.grid(d.path[i]).score;
				}
				v /= d.path.size();
				return v;
			};
			auto finalScore = [&](const SearchData &current, Point np) {
				if (evaluate(current) < 1e-9) return 1e-10;
				Field f_(m_field);
				for (int i = 1; i < current.path.size(); i++) {
					f_.setColor(current.path[i], pId);
				}
				auto res = f_.calcScore();
				double v;
				if (pId == PlayerId::A) v = res.first;
				else v = res.second;
				return v;
			};
			auto isFinal = [&](const SearchData &current, Point np) {
				return e == np;
			};
			SP<SearchData> first(new SearchData());
			first->path.push_back(s);
			first->used[s.y][s.x] = true;
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					auto c = m_field.grid(Point(x, y)).color;
					if (c && *c == pId) first->used[y][x] = true;
				}
			}
			return search({first}, evaluate, finalScore, isFinal, 1, false, distance(s, e)*2);
		}();
		if (path.size() < 2) return;
		bool po[H][W] = {};
		for (int i = 1; i < path.size(); i++) po[path[i].y][path[i].x] = true;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				auto c = m_field.grid(Point(j, i)).color;
				if (c && *c == pId) std::cout << "X ";
				else if (po[i][j]) std::cout << "* ";
				else std::cout << "  ";
			}
			std::cout << std::endl;
		}
	}

	std::pair<std::vector<Point>, std::vector<Point>> calcPath(PlayerId pId) {

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
						if (m_field.outOfField(q)) continue;
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
		test(m_pId);
		return PlayerMove(actions[0], actions[1]);
	}
};



class WinjAI2 : AI {
private:

	Field m_field;

	PlayerId m_pId;

	enum class GridState {
		Original = 0,
		Removed = 1,
		Passed = 2,
	};
	
	struct SearchData {
		GridState gst[Field::MAX_H][Field::MAX_W];
		std::vector<Point> path;
		double v;
		int turn;
		SearchData() {
			for (int i = 0; i < Field::MAX_H; i++) {
				for (int j = 0; j < Field::MAX_W; j++) {
					gst[i][j] = GridState::Original;
				}
			}
			v = 0.0;
			turn = 0;
		}
	};

public:

	WinjAI2() {}

	int distance(Point a, Point b) {
		return std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
	};

	SearchData search(
		PlayerId pId,
		const std::vector<SP<SearchData>> &firsts,
		std::function<double(const SearchData&)> evaluate, //同じ深さの盤面同士を比較できればいい
		std::function<double(const SearchData& current, Point np)> finalScore,
		std::function<bool(const SearchData& current, Point np)> isFinal,
		std::function<bool(const SearchData& current, Point np)> isRemovable,
		std::vector<std::vector<Point>> recommended,
		int beamWidth,
		bool output = false,
		int steps = -1) {
		PlayerId pId_ = (PlayerId)(1 - (int)pId);

		auto similar = [&](const SearchData &a, const SearchData &b) {
			int cnt = 0;
			for (int i = 0; i < m_field.h(); i++) {
				for (int j = 0; j < m_field.w(); j++) {
					cnt += ((a.gst[i][j] == GridState::Passed) != (b.gst[i][j] == GridState::Passed));
				}
			}
			//return cnt < std::min(5, (int)std::min(a.path.size(), b.path.size())/3);
			return cnt == 0;
		};
		auto comp = [&](const SP<SearchData> a, const SP<SearchData> b) {
			return a->v > b->v;
		};
		auto toNext = [&](SearchData &current, Point np) {
			auto color = m_field.grid(np).color;
			int cost = 1;
			if (color && *color == pId_ && current.gst[np.y][np.x] == GridState::Original)
				cost++;
			current.path.push_back(np);
			current.gst[np.y][np.x] = GridState::Passed;
			current.turn += cost;
		};

		SearchData ret;
		double max = -1e10;
		auto updateBest = [&](const SearchData &previous, Point e) {
			double v = finalScore(previous, e);
			if (max < v) {
				max = v;
				ret = previous;
				toNext(ret, e);
			}
		};

		using Vec = std::vector<SP<SearchData>>;
		Vec states[Field::MAX_H*Field::MAX_W][Field::MAX_H][Field::MAX_W];
		/*for (int y = 0; y < field.h(); y++) {
		for (int x = 0; x < field.w(); x++) {
		SP<SearchData> first(new SearchData());
		first->path.push_back(Point(x, y));
		first->used[y][x] = true;
		states[y][x][0].push_back(first);
		}
		}*/
		for (int i = 0; i < firsts.size(); i++) {
			states[0][firsts[i]->path.back().y][firsts[i]->path.back().x].push_back(firsts[i]);	
		}
		if (steps < 0) steps = m_field.h()*m_field.w()/2;
		for (int step = 0; step < steps; step++) {
			if (output) std::cout << "step:" << step << std::endl;
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					int cnt = 0;
					for (int vi = 0; cnt < beamWidth && vi < (int)states[step][y][x].size(); vi++) {
						SP<SearchData> t = states[step][y][x][vi];
						if ([&]() {
							for (int vj = 0; vj < vi; vj++) {
								if (similar(*t, *states[step][y][x][vj]))
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
							if(isFinal(*t, np)) {
								updateBest(*t, np);
								continue;
							}
							SP<SearchData> next(new SearchData(*t));
							toNext(*next, np);
							next->v = evaluate(*next);
							//if (t->gst[np.y][np.x] == GridState::Passed) next->v -= 1000;
							/*if ((dir&1) == 0) {
							Point cross0 = Point(x, y) + Neighbour8((Direction8)((dir + 1) % 8));
							Point cross1 = Point(x, y) + Neighbour8((Direction8)((dir + 7) % 8));
							next->used[cross0.y][cross0.x] = true;
							next->used[cross1.y][cross1.x] = true;
							}*/
							states[next->turn][np.y][np.x].push_back(next);
							for (int ri = 0; ri < recommended.size(); ri++) {
								for (int k = 0; k < 2; k++, std::reverse(recommended[ri].begin(), recommended[ri].end())) {
									if (!(recommended[ri].front() == np)) continue;
									SP<SearchData> next_(new SearchData(*t));
									for (int rj = 0; rj < recommended[ri].size(); rj++) {
										toNext(*next_, recommended[ri][rj]);
									}
									Point e = recommended[ri].back();
									next_->v = evaluate(*next_);
									states[next_->turn][e.y][e.x].push_back(next_);
								}
							}
						}

						for (int dir = 0; dir < 8; dir++) {
							Point np = Point(x, y) + Neighbour8((Direction8)dir);
							if (np.x < 0 || m_field.w() <= np.x || np.y < 0 || m_field.h() <= np.y)
								continue;
							//if (m_field.grid(np).score < 0) continue;
							auto color = m_field.grid(np).color;
							if (!color || *color != pId_ || t->gst[np.y][np.x] != GridState::Original) continue;
							if (!isRemovable(*t, np)) continue;
							
							SP<SearchData> next(new SearchData(*t));
							next->v = evaluate(*next);
							next->gst[np.y][np.x] = GridState::Removed;
							next->turn++;
							states[next->turn][y][x].push_back(next);
						}

					}
					states[step][y][x].clear();
				}
			}
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					auto &v = states[step + 1][y][x];
					std::sort(v.begin(), v.end(), comp);
				}
			}
		}
		return ret;
	}


	std::vector<std::vector<Point>> areaCap(PlayerId pId) {
		const int H = Field::MAX_H;
		const int W = Field::MAX_W;
		const int h = m_field.h();
		const int w = m_field.w();
		const int INT_INF = 1e5;
		auto warshallFloyd = [&](int d[H][W][H][W], const Field &fld) {
			for (int y = 0; y < H; y++) {
				for (int x = 0; x < W; x++) {
					for (int y_ = 0; y_ < H; y_++) {
						for (int x_ = 0; x_ < W; x_++) {
							d[y][x][y_][x_] = INT_INF;
						}
					}
				}
			}
			for (int y = 0; y < H; y++) {
				for (int x = 0; x < W; x++) {
					auto color = fld.grid(Point(x, y)).color;
					if (!(color && *color == pId)) continue;
					d[y][x][y][x] = 0;
					for (int dir = 0; dir < 8; dir++) {
						Point np = Point(x, y) + Neighbour8((Direction8)dir);
						if (fld.outOfField(np)) continue;
						auto c = fld.grid(np).color;
						if (c && *c == pId) d[y][x][np.y][np.x] = 1;
					}
				}
			}
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					for (int y = 0; y < h; y++) {
						for (int x = 0; x < w; x++) {
							for (int y_ = 0; y_ < h; y_++) {
								for (int x_ = 0; x_ < w; x_++) {
									int c = d[y][x][i][j] + d[i][j][y_][x_];
									d[y][x][y_][x_] = std::min(d[y][x][y_][x_], c);
								}
							}
						}
					}
				}
			}
		};
		auto calcPath = [&](int d[H][W][H][W], Point s, Point e){
			auto evaluate = [&](const SearchData &d) {
				bool u[Field::MAX_H][Field::MAX_W] = {};
				for (int i = 0; i < d.path.size(); i++) {
					u[d.path[i].y][d.path[i].x] = true;
				}
				for (int i = 0; i < d.path.size(); i++) {
					int cnt = 0;
					for (int k = 0; k < 8; k++) {
						Point np = d.path[i] + Neighbour8((Direction8)k);
						if (m_field.outOfField(np)) continue;
						if (u[np.y][np.x]) cnt++;
					}
					if (cnt >= 3) return -1e10;
				}
				double v = 0;
				for (int i = 1; i < d.path.size(); i++) {
					v += m_field.grid(d.path[i]).score;
				}
				v /= d.path.size();
				return v;
			};
			auto finalScore = [&](const SearchData &current, Point np) {
				if (evaluate(current) < 1e-9) return 1e-10;
				Field f_(m_field);
				for (int i = 1; i < current.path.size(); i++) {
					f_.setColor(current.path[i], pId);
				}
				auto res = f_.calcScore();
				double v;
				if (pId == PlayerId::A) v = res.first;
				else v = res.second;
				return v;
			};
			auto isFinal = [&](const SearchData &current, Point np) {
				return e == np;
			};
			auto isRemovable = [](const SearchData &current, Point np) {
				return false;
			};
			SP<SearchData> first(new SearchData());
			first->path.push_back(s);
			first->gst[s.y][s.x] = GridState::Passed;
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					auto c = m_field.grid(Point(x, y)).color;
					if (c && *c == pId) first->gst[y][x] = GridState::Passed;
				}
			}
			return search(pId, {first}, evaluate, finalScore, isFinal, isRemovable, {}, 1, false, distance(s, e)*2).path;
		};

		int d[H][W][H][W];
		std::vector<std::vector<Point>> ret;
		int areaScore = m_field.calcAreaScore(pId);

		Field painted(m_field);
		for (int capCount = 0; capCount < 6; capCount++) {
			warshallFloyd(d, painted);
			double max = -1e10;
			Point s, e;
			for (int y = 0; y < h; y++) {
				for (int x = 0; x < w; x++) {
					Point p(x, y);
					for (int y_ = 0; y_ < h; y_++) {
						for (int x_ = 0; x_ < w; x_++) {
							Point p_(x_, y_);
							int dist = distance(p, p_);
							if (dist <= 1 || d[y][x][y_][x_] >= INT_INF) continue;
							double v = d[y][x][y_][x_]/(double)dist;
							if (max < v) {
								max = v;
								s = p;
								e = p_;
							}
						}
					}
				}
			}
			std::vector<Point> path = calcPath(d, s, e);
			if (max < 0) continue;
			if (path.size() < 3) continue;
			path.erase(path.begin()); path.pop_back();

			Field f_(m_field);
			for (int i = 0; i < path.size(); i++) {
				f_.setColor(path[i], pId);
			}
			//領域ポイントが増えなくなったら終わり
			if (areaScore >= f_.calcAreaScore(pId)) break;

			for (int i = 0; i < path.size(); i++) {
				painted.setColor(path[i], pId);
			}
			ret.push_back(path);


			bool tsuyosshi[H][W] = {};
			for (int i = 0; i < path.size(); i++) tsuyosshi[path[i].y][path[i].x] = true;
			std::cout << "area cap " << capCount << ", player " << (int)pId << std::endl;
			for (int i = 0; i < h; i++) {
				for (int j = 0; j < w; j++) {
					auto c = m_field.grid(Point(j, i)).color;
					if (s == Point(j, i)) std::cout << "E ";
					else if (e == Point(j, i)) std::cout << "E ";
					else if (c && *c == pId) std::cout << "X ";
					else if (tsuyosshi[i][j]) std::cout << "* ";
					else std::cout << "  ";
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		return ret;
	}

	PlayerMove po(PlayerId pId) {

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

		auto calcVirtualAreaScore = [&](PlayerId p, const Field &field, const std::vector<std::vector<Point>> &areaCap) {
			Field field_(field);
			for (int i = 0; i < areaCap.size(); i++) {
				for (int j = 0; j < areaCap[i].size(); j++) {
					field_.setColor(areaCap[i][j], p);
				}
			}
			return field_.calcAreaScore(p);
		};

		auto myAreaCap = areaCap(pId);
		auto oppAreaCap = areaCap(pId_);
		bool isMyCap[Field::MAX_H][Field::MAX_W] = {};
		bool isOppCap[Field::MAX_H][Field::MAX_W] = {};
		for (int i = 0; i < myAreaCap.size(); i++) {
			for (int j = 0; j < myAreaCap[i].size(); j++) {
				isMyCap[myAreaCap[i][j].y][myAreaCap[i][j].x] = true;
			}
		}
		for (int i = 0; i < oppAreaCap.size(); i++) {
			for (int j = 0; j < oppAreaCap[i].size(); j++) {
				isOppCap[oppAreaCap[i][j].y][oppAreaCap[i][j].x] = true;
			}
		}
		auto moderateMyAreaScore = [&](const Field &fld, const SearchData &paint) {
			Field f_(fld);
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					if (paint.gst[y][x] == GridState::Removed) f_.setColor(Point(x, y), {});
					else if (paint.gst[y][x] == GridState::Passed && isMyCap[y][x])
						f_.setColor(Point(x, y), pId);
				}
			}
			return f_.calcAreaScore(pId);
		};
		Field origin(m_field);
		std::array<int, 2> originAreaScore = origin.calcAreaScore();
		std::array<int, 2> originNormalScore = origin.calcNormalScore();
		double oppVirtualAreaScore = calcVirtualAreaScore(pId_, origin, oppAreaCap);
		auto setOrigin = [&](const SearchData &paint) {
			origin = m_field;
			originAreaScore[(int)pId] = moderateMyAreaScore(origin, paint);
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					if (paint.gst[y][x] == GridState::Passed) origin.setColor(Point(x, y), pId);
					else if (paint.gst[y][x] == GridState::Removed) origin.setColor(Point(x, y), {});
				}
			}
			originAreaScore[(int)pId_] = origin.calcAreaScore(pId_);
			originNormalScore = origin.calcNormalScore();
			calcVirtualAreaScore(pId_, origin, oppAreaCap);
		};

		int maxLength = 0;
		int minLength = 0;
		auto evaluate = [&](const SearchData &d) {
			Field f(origin);
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					if (d.gst[y][x] == GridState::Removed) f.setColor(Point(x, y), {});
					else if (d.gst[y][x] == GridState::Passed) f.setColor(Point(x, y), pId);
				}
			}
			double myGain = [&] {
				int resTurn = f.resTurn() - d.turn;
				double r = std::max(1 - resTurn/20.0, 0.2);
				auto s = f.calcNormalScore()[(int)pId] + r*moderateMyAreaScore(m_field, d);
				return s - (originNormalScore[(int)pId] + r*originAreaScore[(int)pId]);
				/*auto s = f.calcScore();;
				if (pId == PlayerId::A) return s.first;
				return s.second;*/
			}();
			double oppGain = [&] {
				double normalGain = f.calcNormalScore()[(int)pId_] - originNormalScore[(int)pId_];
				double areaGain = f.calcAreaScore(pId_) - originAreaScore[(int)pId_];
				double virtualAreaGain = calcVirtualAreaScore(pId_, f, oppAreaCap) - oppVirtualAreaScore;
				return normalGain + (areaGain + virtualAreaGain)/2.0;
			}();
			return (myGain - oppGain)/(double)d.turn;
		};
		auto finalScore = [&](const SearchData &current, Point np) {
			if (current.path.size() < minLength || maxLength < current.path.size()) return 1e-10;
			return evaluate(current);
		};
		auto isRemovable = [&](const SearchData &current, Point np) {
			Field f(origin);
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					if (current.gst[y][x] == GridState::Original) continue;
					f.setColor(Point(x, y), {});
				}
			}
			Field f_(f);
			f_.setColor(np, {});
			return f.calcAreaScore(pId_) < f_.calcAreaScore(pId_);
		};
		auto isFinal0 = [&](const SearchData &current, Point np) {
			if (m_field.resTurn() - current.turn - 1 == 0) return true;
			return np == p0_;
		};
		auto isFinal1 = [&](const SearchData &current, Point np) {
			if (m_field.resTurn() - current.turn - 1 == 0) return true;
			return np == p1_;
		};
		auto toAction = [&](const SearchData &result) {
			Point p = result.path.front();
			for (int dir = 0; dir < 8; dir++) {
				Point np = p + Neighbour8((Direction8)dir);
				if (m_field.outOfField(np)) continue;
				if (result.gst[np.y][np.x] == GridState::Removed) {
					return Action(ActionType::Remove, (Direction8)dir);
				}
			}
			for (int dir = 0; dir < 8; dir++) {
				Point np = p + Neighbour8((Direction8)dir);
				if (result.path[1] == np) {
					auto color = m_field.grid(np).color;
					if (color && *color == pId_) return Action(ActionType::Remove, (Direction8)dir);
					return Action(ActionType::Move, (Direction8)dir);
				}
			}
			throw "oh yeah ...";
		};
		auto print = [&](const SearchData &sd) {
			for (int x = 0; x < m_field.w(); x++) std::cout << "==";
			std::cout << std::endl;
			for (int y = 0; y < m_field.h(); y++) {
				for (int x = 0; x < m_field.w(); x++) {
					if (sd.path.front() == Point(x, y)) std::cout << "X ";
					else if (sd.gst[y][x] == GridState::Passed) std::cout << "M ";
					else if (sd.gst[y][x] == GridState::Removed) std::cout << "* ";
					else std::cout << "  ";
				}
				std::cout << std::endl;
			}
		};
		std::array<OptAction, 2> ret;
		{
			maxLength = std::max((int)(distance(p0, p0_)*1.5), distance(p0, p0_) + 5);
			minLength = distance(p0, p0_) + 3;
			SP<SearchData> first(new SearchData());
			first->path.push_back(p0);
			first->gst[p0.y][p0.x] = GridState::Passed;
			auto res = search(
				pId,
				{first},
				evaluate,
				finalScore,
				isFinal0,
				isRemovable,
				myAreaCap,
				1,
				false);
			setOrigin(res);
			print(res);
		}
		{
			maxLength = std::max((int)(m_field.h()*m_field.w()/4.0), distance(p1, p1_) + 5);
			minLength = distance(p1, p1_) + 3;
			SP<SearchData> first(new SearchData());
			first->path.push_back(p1);
			first->gst[p1.y][p1.x] = GridState::Passed;
			auto res = search(
				pId,
				{first},
				evaluate,
				finalScore,
				isFinal1,
				isRemovable,
				myAreaCap,
				1,
				false);
			setOrigin(res);
			ret[1] = toAction(res);
			print(res);
		}
		{
			maxLength = std::max((int)(m_field.h()*m_field.w()/4.0), distance(p0, p0_) + 5);
			minLength = distance(p0, p0_) + 3;
			SP<SearchData> first(new SearchData());
			first->path.push_back(p0);
			first->gst[p0.y][p0.x] = GridState::Passed;
			auto res = search(
				pId,
				{first},
				evaluate,
				finalScore,
				isFinal0,
				isRemovable,
				myAreaCap,
				1,
				false);
			ret[0] = toAction(res);
			print(res);
		}
		return PlayerMove(ret[0], ret[1]);
	}

	virtual void init(const Field & field, PlayerId playerId) override {
		m_field = field;
		m_pId = playerId;
	}

	virtual PlayerMove calcNextMove(std::optional<std::pair<PlayerMove, PlayerMove>> moves) override {
		if (moves) {
			m_field.forward(moves->first, moves->second);
		}
		return po(m_pId);
		/*auto res = (m_field.resTurn() < 0) ? finalProc(m_pId) : calcPath(m_pId);
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
		test(m_pId);
		return PlayerMove(actions[0], actions[1]);*/
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



namespace FirstPhase {


int toColor(PlayerId pId) { return 1 + (int)pId; }

int toColor(const std::optional<PlayerId> &color) {
	if (!color) return 0;
	return toColor(*color);
}

struct SearchData {
	std::vector<Point> path;
	double v;
	int turn;
	SearchData(Point myPos) {
		v = 0.0;
		turn = 0;
		path.push_back(myPos);
	}
};

int simpleAreaScore(int gc[Field::MAX_H][Field::MAX_W], const Field &fld, PlayerId pId) {
	PlayerId pId_ = (PlayerId)(1 - (int)pId);
	constexpr Point dirPoint[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
	int sum[Field::MAX_H*Field::MAX_W] = {};
	int group[Field::MAX_H][Field::MAX_W];
	for (int y = 0; y < fld.h(); y++) {
		for (int x = 0; x < fld.w(); x++) {
			int num = y*fld.w() + x;
			if (gc[y][x] == 0) {
				sum[num] = std::abs(fld.grid(Point(x, y)).score);
				group[y][x] = num;
			}
			else {
				group[y][x] = -1;
				sum[num] = -100000;
			}
		}
	}
	int step = 0;
	while (true) {
		bool invalid[Field::MAX_H*Field::MAX_W] = {};
		for (int y = 0; y < fld.h(); y++) {
			for (int x = 0; x < fld.w(); x++) {
				Point p(x, y);
				if (group[y][x] < 0) continue;
				for (int dir = 0; dir < 4; dir++) {
					Point np = p + dirPoint[dir];
					if (np.y < 0 || fld.h() <= np.y || np.x < 0 || fld.w() <= np.x || gc[np.y][np.x] == toColor(pId_)) {
						invalid[group[y][x]] = true;
						continue;
					}
					if (gc[np.y][np.x] == toColor(pId)) continue;
					if (group[y][x] != group[np.y][np.x]) invalid[group[y][x]] = true;
					if (group[y][x] < group[np.y][np.x]) {
						group[np.y][np.x] = group[y][x];
						sum[group[y][x]] += std::abs(fld.grid(np).score);
					}
				}
			}
		}
		if (++step == 10) {
			int ret = 0;
			for (int y = 0; y < fld.h(); y++) {
				for (int x = 0; x < fld.w(); x++) {
					//int s = sum[y*fld.w() + x];
					//if (s < 0) printf("(%3d, ---) ", group[y][x]);
					//else printf("(%3d, %3d) ", group[y][x], sum[y*fld.w() + x]);
					int num = y*fld.w() + x;
					if (group[y][x] == num && sum[num] >= 0 && invalid[num] == false)
						ret += sum[num];
				}
				//puts("");
			}
			//puts("");
			return ret;
		}
	}
}

SearchData search(
	const Field &fld,
	Point myPos,
	std::function<bool(SearchData &current, Point np)> nextM,
	std::function<bool(const SearchData &a, const SearchData &b)> similar,
	int steps = -1) {
	const int beamWidth = 1;

	auto comp = [&](const SP<SearchData> a, const SP<SearchData> b) {
		return a->v > b->v;
	};

	std::optional<SearchData> ret;
	double max = -1e10;

	using Vec = std::vector<SP<SearchData>>;
	Vec states[Field::MAX_H*Field::MAX_W][Field::MAX_H][Field::MAX_W];
	SP<SearchData> fsd(new SearchData(myPos));
	states[0][myPos.y][myPos.x].push_back(fsd);
	if (steps < 0) steps = fld.h()*fld.w()/2;
	for (int step = 0; step < steps; step++) {
#pragma omp parallel for
		for (int y = 0; y < fld.h(); y++) {
#pragma omp parallel for
			for (int x = 0; x < fld.w(); x++) {
				int cnt = 0;
				for (int vi = 0; cnt < beamWidth && vi < (int)states[step][y][x].size(); vi++) {
					SP<SearchData> t = states[step][y][x][vi];
					if ([&]() {
						for (int vj = 0; vj < vi; vj++) {
							if (similar(*t, *states[step][y][x][vj]))
								return true;
						}
						return false;
					}() ) continue;
					cnt++;

					for (int dir = 0; dir < 8; dir++) {

						Point np = Point(x, y) + Neighbour8((Direction8)dir);
						if (fld.outOfField(np))
							continue;
						//if (fld.grid(np).score < 0) continue;
						SP<SearchData> next(new SearchData(*t));
						if (nextM(*next, np)) {
#pragma omp critical (UPDATE_BEST)
							{
								if (max < next->v) {
									max = next->v;
									ret = *next;
								}
							}
							continue;
						}
#pragma omp critical (CHANGE_STATES)
						{
							states[next->turn][np.y][np.x].push_back(next);
						}
					}
				}
				states[step][y][x].clear();
			}
		}
		for (int y = 0; y < fld.h(); y++) {
			for (int x = 0; x < fld.w(); x++) {
				auto &v = states[step + 1][y][x];
				std::sort(v.begin(), v.end(), comp);
			}
		}
	}
	return *ret;
};

//oppPath.size() >= 1
SearchData calcPath(AgentId aId, const Field &fld, const std::vector<Point> &oppPath, int turn) {
	PlayerId pId = fld.playerOf(aId);
	PlayerId pId_ = (PlayerId)(1 - (int)pId);
	auto similar = [&](const SearchData &a, const SearchData &b) {
		return false;
	};
	auto nextM = [&](SearchData &current, Point np) {
		int gc[Field::MAX_H][Field::MAX_W] = {};
		for (int y = 0; y < fld.h(); y++) {
			for (int x = 0; x < fld.w(); x++) {
				gc[y][x] = toColor(fld.grid(Point(x, y)).color);
			}
		}
		auto getPos = [](const std::vector<Point> &path, int index) {
			if (path.size() <= index) return path.back();
			return path[index];
		};
		auto isRemove = [&gc, getPos](PlayerId pi, int index, const std::vector<Point> &path) {
			PlayerId pi_ =  (PlayerId)(1 - (int)pi);
			Point p = getPos(path, index);
			Point p_ = getPos(path, index + 1);
			if (gc[p_.y][p_.x] == toColor(pi_)) return true;
			return false;
		};
		auto nextPos = [&gc, isRemove, getPos](PlayerId pi, int index, const std::vector<Point> &path) {
			if (isRemove(pi, index, path)) return getPos(path, index);
			return getPos(path, index + 1);
		};
		/*auto simpleAreaScore = [&]() {
			constexpr Point dirPoint[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
			int v = 0;
			for (int y = 0; y < fld.h(); y++) {
				for (int x = 0; x < fld.w(); x++) {
					if (gc[np.y][np.x] != 0) continue;
					auto f = [&](PlayerId pi) {
						for (int i = 0; i < 4; i++) {
							Point np = Point(x, y) + dirPoint[i];
							if (fld.outOfField(np)) return false;
							if (gc[np.y][np.x] != toColor(pi)) return false; 
						}
						return true;
					};
					if (f(pId)) v += fld.grid(Point(x, y)).score;
					if (f(pId_)) v -= fld.grid(Point(x, y)).score;
				}
			}
			return v;
		};*/
		current.path.push_back(np);
		current.v = 0;
		int originMyArea = simpleAreaScore(gc, fld, pId);
		int originOppArea = simpleAreaScore(gc, fld, pId_);
		const double r = 0.93;
		double w = 1, wSum = 0;
		int i = 0, j = 0;
		bool isEnd = false;
		for (current.turn = 0; i + 1 < current.path.size() && current.turn < turn; current.turn++) {
			auto fw = [&](PlayerId pi, int &index, const std::vector<Point> &path) {
				PlayerId pi_ =  (PlayerId)(1 - (int)pi);
				Point trg = getPos(path, index + 1);
				double k = (pId == pi) ? 1 : -1;
				if (current.turn == 0) k *= 1.5;
				if (gc[trg.y][trg.x] != toColor(pi))
					current.v += k*w*fld.grid(trg).score;
				if (gc[trg.y][trg.x] == toColor(pi_)) {
					gc[trg.y][trg.x] = 0;
					return;
				}
				gc[trg.y][trg.x] = toColor(pi);
				if (index + 1 < path.size()) index++;
			};

			bool myIsRemove = isRemove(pId, i, current.path), oppIsRemove = isRemove(pId_, j, oppPath);

			//相手の行動は決め打ちなので、一方的に行動を無効化できてしまう。そこで敵有利にしておく
			//(無効化をコメントアウト & 相手の行動は後打ち)
			//↓

			//pId側が有利すぎるコード
			/*
			//指定先が被って全て無効
			if (myIsRemove == oppIsRemove && getPos(current.path, i + 1) == getPos(oppPath, j + 1)) {
				isEnd = true;
				break;
			}
			if (myIsRemove) fw(pId, i, current.path);
			if (oppIsRemove) fw(pId_, j, oppPath);
			//位置が被ったので移動行為は無効
			if (nextPos(pId, i, current.path) == nextPos(pId_, j, oppPath)) {
				isEnd = true;
				current.turn++;
				break;
			}
			if (!myIsRemove) fw(pId, i, current.path);
			if (!oppIsRemove) fw(pId_, j, oppPath);
			*/

			if (i <= 1) {
				AgentId opp0 = (AgentId)(2 * (int)pId_ + 0);
				AgentId opp1 = (AgentId)(2 * (int)pId_ + 1);
				Point p = getPos(current.path, i + 1);
				Point a = fld.agentPos(opp0);
				Point b = fld.agentPos(opp1);
				if (p == fld.agentPos(opp0) || p == fld.agentPos(opp1)) {
					current.v = -1e5;
					return true;
				}
			}
			//以下の実装: pId_側に忖度
			if (j + 1 >= oppPath.size() && current.path[i + 1] == oppPath[j]) {
				isEnd = true;
				break;
			}
			if (myIsRemove == oppIsRemove && getPos(current.path, i + 1) == getPos(oppPath, j + 1)) {
				if (i == 0) //初動だけ衝突有利
					current.v += w*fld.grid(getPos(current.path, i + 1)).score*1.1;
			}
			else fw(pId, i, current.path);
			fw(pId_, j, oppPath);
			wSum += w;
			w *= r;
		}
		current.v += std::max(simpleAreaScore(gc, fld, pId) - originMyArea, 16)*0.5;
		current.v -= std::max(simpleAreaScore(gc, fld, pId_) - originOppArea, 16)*0.5;
		current.v /= wSum;
		//return isEnd && std::abs(current.turn - turn) <= 3;
		return isEnd || turn - current.turn == 0;
	};
	return search(fld, fld.agentPos(aId), nextM, similar, turn);
}

std::array<SearchData, 2> calcPath(PlayerId pId, const Field &fld, bool swapped, std::array<std::optional<SearchData>, 2> oppData = {}) {
	PlayerId pId_ = (PlayerId)(1 - (int)pId);
	AgentId a0 = (AgentId)(2*(int)pId + 0);
	AgentId a1 = (AgentId)(2*(int)pId + 1);
	AgentId a0_ = (AgentId)(2*(int)pId_ + 0);
	AgentId a1_ = (AgentId)(2*(int)pId_ + 1);
	if (swapped) {
		std::swap(a0, a1);
		std::swap(a0_, a1_);
	}
	Point p0 = fld.agentPos(a0);
	Point p1 = fld.agentPos(a1);
	Point p0_ = fld.agentPos(a0_);
	Point p1_ = fld.agentPos(a1_);
	if (!oppData[0]) oppData[0] = SearchData(p0_);
	if (!oppData[1]) oppData[1] = SearchData(p1_);
	auto &oppData0 = oppData[(int)a0_ - 2*(int)pId_];
	auto &oppData1 = oppData[(int)a1_ - 2*(int)pId_];

	Field origin(fld);
	auto setOrigin = [&](const SearchData &paint) {
		origin = fld;
		for (int i = 0; i < paint.path.size(); i++) {
			origin.setColor(paint.path[i], pId);
		}
	};

	std::array<std::optional<SearchData>, 2> ret;
	for (int turn = std::min(4, fld.resTurn()); turn <= std::min(15, fld.resTurn()); turn += 4) {
		std::cout << turn << std::endl;
		ret[0] = calcPath(a0, origin, oppData0->path, turn);
		setOrigin(*ret[0]);
		ret[1] = calcPath(a1, origin, oppData1->path, turn);
		setOrigin(*ret[1]);
		if (turn != 12) continue;
		std::cout << turn << "========================" << std::endl;
		bool used0[Field::MAX_H][Field::MAX_W] = {};
		bool used1[Field::MAX_H][Field::MAX_W] = {};
		for (int i = 0; i < ret[0]->path.size(); i++)
			used0[ret[0]->path[i].y][ret[0]->path[i].x] = true;
		for (int i = 0; i < ret[1]->path.size(); i++)
			used1[ret[1]->path[i].y][ret[1]->path[i].x] = true;
		for (int y = 0; y < fld.h(); y++) {
			for (int x = 0; x < fld.w(); x++) {
				if (Point(x, y) == p0) std::cout << "X ";
				else if (Point(x, y) == p1) std::cout << "X ";
				else if (used0[y][x] && used1[y][x]) std::cout << "* ";
				else if (used0[y][x]) std::cout << "0 ";
				else if (used1[y][x]) std::cout << "1 ";
				else std::cout << "  ";
			}
			std::cout << std::endl;
		}
		std::cout << ret[0]->v << " + " << ret[1]->v << " = " << (ret[0]->v + ret[1]->v)/2.0 << std::endl;
	}
	if (swapped) return {*ret[1], *ret[0]};
	return {*ret[0], *ret[1]};
}

PlayerMove solve(PlayerId pId, const Field &fld) {
	PlayerId pId_ = (PlayerId)(1 - (int)pId);

	int gc[Field::MAX_H][Field::MAX_W] = {};
	for (int y = 0; y < fld.h(); y++) {
		for (int x = 0; x < fld.w(); x++) {
			gc[y][x] = toColor(fld.grid(Point(x, y)).color);
		}
	}
	int area0 = simpleAreaScore(gc, fld, PlayerId::A);
	int area1 = simpleAreaScore(gc, fld, PlayerId::B);
	std::cout << area0 << ", " << area1 << std::endl;

	auto wrapper = [&](PlayerId pId, const std::array<std::optional<SearchData>, 2> &oppData = {}) {
		auto res0 = calcPath(pId, fld, false, oppData);
		auto res1 = calcPath(pId, fld, true, oppData);
		double v0 = (res0[0].v + res0[1].v)/2.0;
		double v1 = (res1[0].v + res1[1].v)/2.0;
		if (v0 > v1) return res0;
		return res1;
	};
	auto toAction = [&](const SearchData &result) {
		Point p = result.path.front();
		for (int dir = 0; dir < 8; dir++) {
			Point np = p + Neighbour8((Direction8)dir);
			if (result.path[1] == np) {
				auto color = fld.grid(np).color;
				if (color && *color == pId_) return Action(ActionType::Remove, (Direction8)dir);
				return Action(ActionType::Move, (Direction8)dir);
			}
		}
		throw "oh yeah ...";
	};
	auto res = wrapper(pId_, {});
	res = wrapper(pId, {res[0], res[1]});
	return PlayerMove(toAction(res[0]), toAction(res[1]));
}


}


namespace FirstPhase2 {


int toColor(PlayerId pId) { return 1 + (int)pId; }

int toColor(const std::optional<PlayerId> &color) {
	if (!color) return 0;
	return toColor(*color);
}

struct SearchData {
	std::vector<Point> path;
	double v;
	int turn;
	SearchData(Point myPos) {
		v = 0.0;
		turn = 0;
		path.push_back(myPos);
	}
};

int simpleAreaScore(int gc[Field::MAX_H][Field::MAX_W], const Field &fld, PlayerId pId) {
	PlayerId pId_ = (PlayerId)(1 - (int)pId);
	constexpr Point dirPoint[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
	int sum[Field::MAX_H*Field::MAX_W] = {};
	int group[Field::MAX_H][Field::MAX_W];
	for (int y = 0; y < fld.h(); y++) {
		for (int x = 0; x < fld.w(); x++) {
			int num = y*fld.w() + x;
			if (gc[y][x] == 0) {
				sum[num] = std::abs(fld.grid(Point(x, y)).score);
				group[y][x] = num;
			}
			else {
				group[y][x] = -1;
				sum[num] = -100000;
			}
		}
	}
	int step = 0;
	while (true) {
		bool invalid[Field::MAX_H*Field::MAX_W] = {};
		for (int y = 0; y < fld.h(); y++) {
			for (int x = 0; x < fld.w(); x++) {
				Point p(x, y);
				if (group[y][x] < 0) continue;
				for (int dir = 0; dir < 4; dir++) {
					Point np = p + dirPoint[dir];
					if (np.y < 0 || fld.h() <= np.y || np.x < 0 || fld.w() <= np.x || gc[np.y][np.x] == toColor(pId_)) {
						invalid[group[y][x]] = true;
						continue;
					}
					if (gc[np.y][np.x] == toColor(pId)) continue;
					if (group[y][x] != group[np.y][np.x]) invalid[group[y][x]] = true;
					if (group[y][x] < group[np.y][np.x]) {
						group[np.y][np.x] = group[y][x];
						sum[group[y][x]] += std::abs(fld.grid(np).score);
					}
				}
			}
		}
		if (++step == 10) {
			int ret = 0;
			for (int y = 0; y < fld.h(); y++) {
				for (int x = 0; x < fld.w(); x++) {
					//int s = sum[y*fld.w() + x];
					//if (s < 0) printf("(%3d, ---) ", group[y][x]);
					//else printf("(%3d, %3d) ", group[y][x], sum[y*fld.w() + x]);
					int num = y*fld.w() + x;
					if (group[y][x] == num && sum[num] >= 0 && invalid[num] == false)
						ret += sum[num];
				}
				//puts("");
			}
			//puts("");
			return ret;
		}
	}
}

SearchData search(
	const Field &fld,
	Point myPos,
	std::function<bool(SearchData &current, Point np)> nextM,
	std::function<bool(const SearchData &a, const SearchData &b)> similar,
	int steps = -1) {
	const int beamWidth = 1;

	auto comp = [&](const SP<SearchData> a, const SP<SearchData> b) {
		return a->v > b->v;
	};

	std::optional<SearchData> ret;
	double max = -1e10;

	using Vec = std::vector<SP<SearchData>>;
	Vec states[Field::MAX_H*Field::MAX_W][Field::MAX_H][Field::MAX_W];
	SP<SearchData> fsd(new SearchData(myPos));
	states[0][myPos.y][myPos.x].push_back(fsd);
	if (steps < 0) steps = fld.h()*fld.w()/2;
	for (int step = 0; step < steps; step++) {
#pragma omp parallel for
		for (int y = 0; y < fld.h(); y++) {
#pragma omp parallel for
			for (int x = 0; x < fld.w(); x++) {
				int cnt = 0;
				for (int vi = 0; cnt < beamWidth && vi < (int)states[step][y][x].size(); vi++) {
					SP<SearchData> t = states[step][y][x][vi];
					if ([&]() {
						for (int vj = 0; vj < vi; vj++) {
							if (similar(*t, *states[step][y][x][vj]))
								return true;
						}
						return false;
					}()) continue;
					cnt++;

					for (int dir = 0; dir < 8; dir++) {

						Point np = Point(x, y) + Neighbour8((Direction8)dir);
						if (fld.outOfField(np))
							continue;
						//if (fld.grid(np).score < 0) continue;
						SP<SearchData> next(new SearchData(*t));
						if (nextM(*next, np)) {
#pragma omp critical (UPDATE_BEST)
							{
								if (max < next->v) {
									max = next->v;
									ret = *next;
								}
							}
							continue;
						}
#pragma omp critical (CHANGE_STATES)
						{
							states[next->turn][np.y][np.x].push_back(next);
						}
					}
				}
				states[step][y][x].clear();
			}
		}
		for (int y = 0; y < fld.h(); y++) {
			for (int x = 0; x < fld.w(); x++) {
				auto &v = states[step + 1][y][x];
				std::sort(v.begin(), v.end(), comp);
			}
		}
	}
	return *ret;
};

//oppPath.size() >= 1
SearchData calcPath(AgentId aId, const Field &fld, const std::vector<Point> &oppPath, int turn) {
	PlayerId pId = fld.playerOf(aId);
	PlayerId pId_ = (PlayerId)(1 - (int)pId);
	auto similar = [&](const SearchData &a, const SearchData &b) {
		return false;
	};
	auto nextM = [&](SearchData &current, Point np) {
		int gc[Field::MAX_H][Field::MAX_W] = {};
		for (int y = 0; y < fld.h(); y++) {
			for (int x = 0; x < fld.w(); x++) {
				gc[y][x] = toColor(fld.grid(Point(x, y)).color);
			}
		}
		auto getPos = [](const std::vector<Point> &path, int index) {
			if (path.size() <= index) return path.back();
			return path[index];
		};
		auto isRemove = [&gc, getPos](PlayerId pi, int index, const std::vector<Point> &path) {
			PlayerId pi_ = (PlayerId)(1 - (int)pi);
			Point p = getPos(path, index);
			Point p_ = getPos(path, index + 1);
			if (gc[p_.y][p_.x] == toColor(pi_)) return true;
			return false;
		};
		auto nextPos = [&gc, isRemove, getPos](PlayerId pi, int index, const std::vector<Point> &path) {
			if (isRemove(pi, index, path)) return getPos(path, index);
			return getPos(path, index + 1);
		};
		/*auto simpleAreaScore = [&]() {
			constexpr Point dirPoint[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
			int v = 0;
			for (int y = 0; y < fld.h(); y++) {
				for (int x = 0; x < fld.w(); x++) {
					if (gc[np.y][np.x] != 0) continue;
					auto f = [&](PlayerId pi) {
						for (int i = 0; i < 4; i++) {
							Point np = Point(x, y) + dirPoint[i];
							if (fld.outOfField(np)) return false;
							if (gc[np.y][np.x] != toColor(pi)) return false;
						}
						return true;
					};
					if (f(pId)) v += fld.grid(Point(x, y)).score;
					if (f(pId_)) v -= fld.grid(Point(x, y)).score;
				}
			}
			return v;
		};*/
		current.path.push_back(np);
		current.v = 0;
		int originMyArea = simpleAreaScore(gc, fld, pId);
		int originOppArea = simpleAreaScore(gc, fld, pId_);
		const double r = 0.9;
		double w = 1, wSum = 0;
		int i = 0, j = 0;
		bool isEnd = false;
		for (current.turn = 0; i + 1 < current.path.size() && current.turn < turn; current.turn++) {
			auto fw = [&](PlayerId pi, int &index, const std::vector<Point> &path) {
				PlayerId pi_ = (PlayerId)(1 - (int)pi);
				Point trg = getPos(path, index + 1);
				double k = (pId == pi) ? 1 : -1;
				if (current.turn == 0) k *= 1.5;
				if (gc[trg.y][trg.x] != toColor(pi))
					current.v += k*w*fld.grid(trg).score;
				if (gc[trg.y][trg.x] == toColor(pi_)) {
					gc[trg.y][trg.x] = 0;
					return;
				}
				gc[trg.y][trg.x] = toColor(pi);
				if (index + 1 < path.size()) index++;
			};

			bool myIsRemove = isRemove(pId, i, current.path), oppIsRemove = isRemove(pId_, j, oppPath);

			//相手の行動は決め打ちなので、一方的に行動を無効化できてしまう。そこで敵有利にしておく
			//(無効化をコメントアウト & 相手の行動は後打ち)
			//↓

			//pId側が有利すぎるコード
			/*
			//指定先が被って全て無効
			if (myIsRemove == oppIsRemove && getPos(current.path, i + 1) == getPos(oppPath, j + 1)) {
				isEnd = true;
				break;
			}
			if (myIsRemove) fw(pId, i, current.path);
			if (oppIsRemove) fw(pId_, j, oppPath);
			//位置が被ったので移動行為は無効
			if (nextPos(pId, i, current.path) == nextPos(pId_, j, oppPath)) {
				isEnd = true;
				current.turn++;
				break;
			}
			if (!myIsRemove) fw(pId, i, current.path);
			if (!oppIsRemove) fw(pId_, j, oppPath);
			*/

			if (i <= 1) {
				AgentId opp0 = (AgentId)(2 * (int)pId_ + 0);
				AgentId opp1 = (AgentId)(2 * (int)pId_ + 1);
				Point p = getPos(current.path, i + 1);
				Point a = fld.agentPos(opp0);
				Point b = fld.agentPos(opp1);
				if (p == fld.agentPos(opp0) || p == fld.agentPos(opp1)) {
					current.v = -1e5;
					return true;
				}
			}
			//以下の実装: pId_側に忖度
			if (j + 1 >= oppPath.size() && current.path[i + 1] == oppPath[j]) {
				isEnd = true;
				break;
			}
			if (myIsRemove == oppIsRemove && getPos(current.path, i + 1) == getPos(oppPath, j + 1)) {
				if (i == 0) //初動だけ衝突有利
					current.v += w*fld.grid(getPos(current.path, i + 1)).score*1.1;
			}
			else fw(pId, i, current.path);
			fw(pId_, j, oppPath);
			wSum += w;
			w *= r;
		}
		current.v += std::max(simpleAreaScore(gc, fld, pId) - originMyArea, 16)*0.5;
		current.v -= std::max(simpleAreaScore(gc, fld, pId_) - originOppArea, 16)*0.5;
		current.v /= wSum;
		//return isEnd && std::abs(current.turn - turn) <= 3;
		return isEnd || turn - current.turn == 0;
	};
	return search(fld, fld.agentPos(aId), nextM, similar, turn);
}

std::array<SearchData, 2> calcPath(PlayerId pId, const Field &fld, bool swapped, std::array<std::optional<SearchData>, 2> oppData = {}) {
	PlayerId pId_ = (PlayerId)(1 - (int)pId);
	AgentId a0 = (AgentId)(2*(int)pId + 0);
	AgentId a1 = (AgentId)(2*(int)pId + 1);
	AgentId a0_ = (AgentId)(2*(int)pId_ + 0);
	AgentId a1_ = (AgentId)(2*(int)pId_ + 1);
	if (swapped) {
		std::swap(a0, a1);
		std::swap(a0_, a1_);
	}
	Point p0 = fld.agentPos(a0);
	Point p1 = fld.agentPos(a1);
	Point p0_ = fld.agentPos(a0_);
	Point p1_ = fld.agentPos(a1_);
	if (!oppData[0]) oppData[0] = SearchData(p0_);
	if (!oppData[1]) oppData[1] = SearchData(p1_);
	auto &oppData0 = oppData[(int)a0_ - 2*(int)pId_];
	auto &oppData1 = oppData[(int)a1_ - 2*(int)pId_];

	Field origin(fld);
	auto setOrigin = [&](const SearchData &paint) {
		origin = fld;
		for (int i = 0; i < paint.path.size(); i++) {
			origin.setColor(paint.path[i], pId);
		}
	};

	std::array<std::optional<SearchData>, 2> ret;
	for (int turn = std::min(2, fld.resTurn()); turn <= std::min(18, fld.resTurn()); turn += 3) {
		std::cout << turn << std::endl;
		ret[0] = calcPath(a0, origin, oppData0->path, turn);
		setOrigin(*ret[0]);
		ret[1] = calcPath(a1, origin, oppData1->path, turn);
		setOrigin(*ret[1]);
		if (turn != 12) continue;
		std::cout << turn << "========================" << std::endl;
		bool used0[Field::MAX_H][Field::MAX_W] = {};
		bool used1[Field::MAX_H][Field::MAX_W] = {};
		for (int i = 0; i < ret[0]->path.size(); i++)
			used0[ret[0]->path[i].y][ret[0]->path[i].x] = true;
		for (int i = 0; i < ret[1]->path.size(); i++)
			used1[ret[1]->path[i].y][ret[1]->path[i].x] = true;
		for (int y = 0; y < fld.h(); y++) {
			for (int x = 0; x < fld.w(); x++) {
				if (Point(x, y) == p0) std::cout << "X ";
				else if (Point(x, y) == p1) std::cout << "X ";
				else if (used0[y][x] && used1[y][x]) std::cout << "* ";
				else if (used0[y][x]) std::cout << "0 ";
				else if (used1[y][x]) std::cout << "1 ";
				else std::cout << "  ";
			}
			std::cout << std::endl;
		}
		std::cout << ret[0]->v << " + " << ret[1]->v << " = " << (ret[0]->v + ret[1]->v)/2.0 << std::endl;
	}
	if (swapped) return { *ret[1], *ret[0] };
	return { *ret[0], *ret[1] };
}

PlayerMove solve(PlayerId pId, const Field &fld) {
	PlayerId pId_ = (PlayerId)(1 - (int)pId);

	int gc[Field::MAX_H][Field::MAX_W] = {};
	for (int y = 0; y < fld.h(); y++) {
		for (int x = 0; x < fld.w(); x++) {
			gc[y][x] = toColor(fld.grid(Point(x, y)).color);
		}
	}
	int area0 = simpleAreaScore(gc, fld, PlayerId::A);
	int area1 = simpleAreaScore(gc, fld, PlayerId::B);
	std::cout << area0 << ", " << area1 << std::endl;

	auto wrapper = [&](PlayerId pId, const std::array<std::optional<SearchData>, 2> &oppData = {}) {
		auto res0 = calcPath(pId, fld, false, oppData);
		auto res1 = calcPath(pId, fld, true, oppData);
		double v0 = (res0[0].v + res0[1].v)/2.0;
		double v1 = (res1[0].v + res1[1].v)/2.0;
		if (v0 > v1) return res0;
		return res1;
	};
	auto toAction = [&](const SearchData &result) {
		Point p = result.path.front();
		for (int dir = 0; dir < 8; dir++) {
			Point np = p + Neighbour8((Direction8)dir);
			if (result.path[1] == np) {
				auto color = fld.grid(np).color;
				if (color && *color == pId_) return Action(ActionType::Remove, (Direction8)dir);
				return Action(ActionType::Move, (Direction8)dir);
			}
		}
		throw "oh yeah ...";
	};
	auto res = wrapper(pId_, {});
	res = wrapper(pId, { res[0], res[1] });
	return PlayerMove(toAction(res[0]), toAction(res[1]));
}


}




class WinjAI3 : AI {
private:

	Field m_field;

	PlayerId m_pId;

public:

	WinjAI3() {}

	virtual void init(const Field & field, PlayerId playerId) override {
		m_field = field;
		m_pId = playerId;
	}

	virtual PlayerMove calcNextMove(std::optional<std::pair<PlayerMove, PlayerMove>> moves) override {
		if (moves) {
			m_field.forward(moves->first, moves->second);
		}
		PlayerId pId = m_pId, pId_ = (PlayerId)(1 - (int)pId);
		/*auto toAction = [&](const SearchData &result) {
			Point p = result.path.front();
			for (int dir = 0; dir < 8; dir++) {
				Point np = p + Neighbour8((Direction8)dir);
				if (m_field.outOfField(np)) continue;
				if (result.gst[np.y][np.x] == GridState::Removed) {
					return Action(ActionType::Remove, (Direction8)dir);
				}
			}
			for (int dir = 0; dir < 8; dir++) {
				Point np = p + Neighbour8((Direction8)dir);
				if (result.path[1] == np) {
					auto color = m_field.grid(np).color;
					if (color && *color == pId_) return Action(ActionType::Remove, (Direction8)dir);
					return Action(ActionType::Move, (Direction8)dir);
				}
			}
			throw "oh yeah ...";
		};*/
		return FirstPhase::solve(pId, m_field);
		//return PlayerMove(toAction(myRes[0]), toAction(myRes[1]));
		/*auto res = (m_field.resTurn() < 0) ? finalProc(m_pId) : calcPath(m_pId);
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
		test(m_pId);
		return PlayerMove(actions[0], actions[1]);*/
	}
};

class WinjAI4 : AI {
private:

	Field m_field;

	PlayerId m_pId;

public:

	WinjAI4() {}

	virtual void init(const Field & field, PlayerId playerId) override {
		m_field = field;
		m_pId = playerId;
	}

	virtual PlayerMove calcNextMove(std::optional<std::pair<PlayerMove, PlayerMove>> moves) override {
		if (moves) {
			m_field.forward(moves->first, moves->second);
		}
		PlayerId pId = m_pId, pId_ = (PlayerId)(1 - (int)pId);
		/*auto toAction = [&](const SearchData &result) {
			Point p = result.path.front();
			for (int dir = 0; dir < 8; dir++) {
				Point np = p + Neighbour8((Direction8)dir);
				if (m_field.outOfField(np)) continue;
				if (result.gst[np.y][np.x] == GridState::Removed) {
					return Action(ActionType::Remove, (Direction8)dir);
				}
			}
			for (int dir = 0; dir < 8; dir++) {
				Point np = p + Neighbour8((Direction8)dir);
				if (result.path[1] == np) {
					auto color = m_field.grid(np).color;
					if (color && *color == pId_) return Action(ActionType::Remove, (Direction8)dir);
					return Action(ActionType::Move, (Direction8)dir);
				}
			}
			throw "oh yeah ...";
		};*/
		return FirstPhase2::solve(pId, m_field);
		//return PlayerMove(toAction(myRes[0]), toAction(myRes[1]));
		/*auto res = (m_field.resTurn() < 0) ? finalProc(m_pId) : calcPath(m_pId);
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
		test(m_pId);
		return PlayerMove(actions[0], actions[1]);*/
	}
};


}
/*
・TODO: finalScoreでもcnt>=3の計算

・新探索
	・領域を作るための提案経路を引数に取る
	・自チームの提案経路によってショートカットするような処理を仕込む
	・評価時に相手側の提案経路を塗りつぶして領域ポイントを計算し、その領域ポイントと素の領域ポイントの平均とかでほげ（ゲーム終了盤面だったら前者の重み0。終了に近づくほどになだらかに重みを変化させたいが、恐らく後回しでよい）
	・自分の領域ポイントは普通にそのまま計算する（ショートカットが補ってくれる）
	・周り8マス壊してみて、領域ポイントがいい感じに減るなら壊す（一番最初に壊す可能性も当然考慮するべき）
	・ゲーム終了を見据えた探索では、相手を追いかける必要はない（が後から探索するエージェントのことを考えづらくなるので、追いかける実装でも良いかも）
	→終盤は、相手エージェントをゴールとするのではなく任意の座標をゴールにして最大を取れば良いのでは
	・ゲーム終了間近で領域の中を塗って点数を稼いだ気にならないようにしないと

・提案経路について
	・今の(タイル上移動距離)/(単純な距離)を最大化する選び方でもそこそこいいが、ちゃんと領域ポイントも使って評価したい
	→提案経路を置いてみてから領域ポイントを計算し、置く前の領域ポイントとの差分でほげ
	・複数列挙するには？
	→距離の比のやつを計算する時は既に挙げた経路を塗っておく（同じような経路を検出しないように）。領域ポイントを計算するときは、既に挙げた経路を塗ったものとの差分、塗ってないものとの差分を取り、小さい方を評価に使う（自力で領域が作れないならだめ。他の経路が作った領域とかぶってもだめ。みたいなお気持ち）

・不利状況で背後を付けられるとほぼ負ける→不利状況で衝突しに行くこと自体が自殺行為？
TODO: 8 -> 4
止まってる相手（最初の探索時の相手）に突っ込んで無理やり終わらせるやつやめてほしい
*/