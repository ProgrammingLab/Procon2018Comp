#include "Field.h"
#include "MockQueue.h"
#include "Bank.h"


namespace Procon2018 {


using IntQ = MockQueue<int, Field::MAX_H*Field::MAX_W*8>;

int Field::ApproximateGause(int n, int start, int end) {
	int sum = 0;
	for (int i = 0; i < n; i++) {
		sum += Rand::Next(start, end);
	}
	double mean = (double)sum/n;
	return (int)(mean + 0.5 - (mean < 0));
}

Field Field::RandomState(bool gachi) {
	int dh = ApproximateGause(2, -MAX_H + 5, MAX_H - 5 + 1);
	int dw = ApproximateGause(2, -MAX_W + 5, MAX_W - 5 + 1);
	int h = MAX_H - abs(dh);
	int w = MAX_W - abs(dw);
	if (gachi) {
		h = MAX_H;
		w = MAX_W;
	}
	int n = Rand::Next(1, 4 + 1);
	double minusP = Rand::DNext()*0.3;
	int maxScore = Rand::Next(1, 16 + 1);
	if (gachi) maxScore = Rand::Next(10, 16 + 1);

	std::vector<std::vector<Grid>> fld;
	fld.resize(h);
	for (int i = 0; i < h; i++) {
		fld[i].resize(w);
		for (int j = 0; j < w; j++) {
			if (i >= (h + 1)/2) { //中央を境に線対称
				fld[i][j].score = fld[h - 1 - i][j].score;
				continue;
			}
			fld[i][j].score = std::abs(ApproximateGause(n, -maxScore, maxScore + 1));
			if (Rand::DNext() < minusP)
				fld[i][j].score *= -1;
		}
	}
	std::vector<Point> candidate;
	for (int i = 0; i < h/2; i++) {
		for (int j = 0; j < w; j++) {
			candidate.push_back(Point(j, i));
		}
	}
	
	//shuffle
	for (int i = 0; i < (int)candidate.size(); i++) {
		std::swap(candidate[i], candidate[Rand::Next(i + 1)]);
	}
	
	std::array<Point, 4> pos;
	pos[0] = candidate[0];
	pos[1] = candidate[1];
	pos[2] = Point(pos[0].x, h - 1 - pos[0].y);
	pos[3] = Point(pos[1].x, h - 1 - pos[1].y);
	if (Rand::Next(2)) std::swap(pos[0], pos[2]);
	for (int i = 0; i < 4; i++) {
		fld[pos[i].y][pos[i].x].color = std::make_optional(i < 2 ? PlayerId::A : PlayerId::B);
	}

	//debug output
	/*for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			if (fld[i][j].score >= 0) std::cout << " ";
			std::cout << fld[i][j].score << " ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			std::cout << (fld[i][j].color ? (fld[i][j].color.value() == PlayerId::A ? "A" : "B") : "x"); 
		}
		std::cout << std::endl;
	}
	std::cout << std::endl << std::endl;*/

	//ここまでは列方向の対称しか考えていないので、1/2の確率で転置
	if (Rand::Next(2)) {
		std::swap(h, w);
		std::vector<std::vector<Grid>> fld_;
		fld_.resize(h);
		for (int i = 0; i < h; i++) {
			fld_[i].resize(w);
			for (int j = 0; j < w; j++) {
				fld_[i][j] = fld[j][i];
			}
		}
		std::swap(fld, fld_);
		for (int i = 0; i < 4; i++) {
			std::swap(pos[i].x, pos[i].y);
		}
	}

	int resTurn = Rand::Next(60, 120 + 1);
	return Field(resTurn, h, w, fld, pos);
}

Field Field::FromPTree(const boost::property_tree::ptree & pt) {
	auto toInt = [](const std::string& s){
		int ret;
		std::stringstream ss(s);
		ss >> ret;
		return ret;
	};

	using namespace boost::property_tree;
	int resTurn = toInt(pt.get<std::string>("resTurn"));
	int h = toInt(pt.get<std::string>("h"));
	int w = toInt(pt.get<std::string>("w"));
	std::vector<std::vector<int>> score;
	for (auto row : pt.get_child("score")) {
		score.push_back({});
		for (auto unit : row.second) {
			std::string s = unit.second.data();
			score.back().push_back(toInt(s));
		}
	}
	std::vector<std::vector<std::optional<PlayerId>>> color;
	for (auto row : pt.get_child("color")) {
		color.push_back({});
		for (auto unit : row.second) {
			std::string s = unit.second.data();
			if (s == "1")
				color.back().push_back(std::optional<PlayerId>(PlayerId::A));
			else if (s == "2")
				color.back().push_back(std::optional<PlayerId>(PlayerId::B));
			else
				color.back().push_back(std::optional<PlayerId>());
		}
	}
	std::vector<std::vector<Grid>> fld;
	for (int i = 0; i < h; i++) {
		fld.push_back({});
		for (int j = 0; j < w; j++) {
			Grid g;
			g.score = score[i][j];
			g.color = color[i][j];
			fld.back().push_back(g);
		}
	}
	std::vector<Point> pos;
	for (auto elem : pt.get_child("pos")) {
		int x = toInt(elem.second.get<std::string>("x"));
		int y = toInt(elem.second.get<std::string>("y"));
		pos.push_back(Point(x, y));
	}
	std::array<Point, 4> pos_;
	for (int i = 0; i < 4; i++) pos_[i] = pos[i];
	return Field(resTurn, h, w, fld, pos_);
}

Field::Field()
: m_resTurn(0)
, m_w(0)
, m_h(0)
, m_field()
, m_agent{} {}

Field::Field(int resTurn, int h, int w, const std::vector<std::vector<Grid>>& field, const std::array<Point, 4>& agent)
: m_resTurn(resTurn)
, m_w(w)
, m_h(h)
, m_field()
, m_agent() {
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			m_field[i][j] = field[i][j];
		}
	}
	for (int i = 0; i < 4; i++) {
		m_agent[i] = agent[i];
	}
}

int Field::resTurn() const {
	return m_resTurn;
}

int Field::w() const {
	return m_w;
}

int Field::h() const {
	return m_h;
}

bool Field::isEnd() const {
	return m_resTurn == 0;
}

void Field::setResTurn(int resTurn) {
	m_resTurn = resTurn;
}

const Grid& Field::grid(const Point &pos) const {
	return m_field[pos.y][pos.x];
}

void Field::setColor(const Point & pos, const std::optional<PlayerId> &color) {
	m_field[pos.y][pos.x].color = color;
}

void Field::setPos(AgentId agentId, const Point & pos) {
	m_agent[(int)agentId] = pos;
}

const Point& Field::agentPos(AgentId playerId) const {
	return m_agent[(int)playerId];
}

PlayerId Field::playerOf(AgentId playerId) const {
	return (int)playerId < 2 ? PlayerId::A : PlayerId::B;
}

bool Field::outOfField(const Point &pos) const {
	return pos.x < 0 || m_w <= pos.x || pos.y < 0 || m_h <= pos.y;
}

std::pair<int, int> Field::calcScore() const {
	auto normal = calcNormalScore();
	auto area = calcAreaScore();
	return std::make_pair(normal[0] + area[0], normal[1] + area[1]);
}

int Field::calcAreaScore(PlayerId pId) const {
	constexpr Point dirPoint[4] = { {1, 0}, {0, 1}, {-1, 0}, {0, -1} };
	const int O = 1;
	bool used[Field::MAX_H + 2][Field::MAX_W + 2] = {};

	auto toInt = [](const Point &p) {
		const int w = Field::MAX_W + 2;
		return (p.y + 1)*w + (p.x + 1);
	};
	auto toPoint = [](int i) {
		const int w = Field::MAX_W + 2;
		return Point(i%w - 1, i/w - 1);
	};

	SP<IntQ> q = Bank<IntQ>::Allocate();
	q->push(toInt({-1, -1}));
	while (q->size() > 0) {
		const Point f = toPoint(q->pop());
		if (used[f.y + O][f.x + O]) continue;
		used[f.y + O][f.x + O] = true;
		for (int i = 0; i < 4; i++) {
			const Point n = f + dirPoint[i];
			if (n.x < -1 || m_w + 1 <= n.x || n.y < -1 || m_h + 1 <= n.y)
				continue;
			if (used[n.y + O][n.x + O]) continue;
			if (!outOfField(n))
				if (auto &c = m_field[n.y][n.x].color)
					if (c.value() == pId) continue;
			q->push(toInt(n));
		}
	}
	Bank<IntQ>::Release(q);

	int ret = 0;
	for (int y = 0; y < m_h; y++) for (int x = 0; x < m_w; x++) {
		if (used[y + O][x + O]) continue;
		if (auto &c = m_field[y][x].color)
			if (c.value() == pId) continue;
		ret += std::abs(m_field[y][x].score);
	}
	return ret;
}

std::array<int, 2> Field::calcAreaScore() const {
	return std::array<int, 2>{ calcAreaScore(PlayerId::A), calcAreaScore(PlayerId::B) };
}

std::array<int, 2> Field::calcNormalScore() const {
	std::array<int, 2> ret{ 0, 0 };
	for (int y = 0; y < m_h; y++) for (int x = 0; x < m_w; x++) {
		const auto &c = m_field[y][x].color;
		if (!c) continue;
		ret[(int)*c] += m_field[y][x].score;
	}
	return ret;
}

bool Field::checkValid(AgentId agentId, const Action & a) const {
	if (a.type == ActionType::Move) {
		Point next = m_agent[(int)agentId] + Neighbour8(a.dir);
		if (outOfField(next)) return false;
		if (auto &c = m_field[next.y][next.x].color)
			if (c.value() != playerOf(agentId)) return false;
		return true;
	}
	if (a.type == ActionType::Remove) {
		Point target = m_agent[(int)agentId] + Neighbour8(a.dir);
		if (outOfField(target)) return false;
		if (auto &c = m_field[target.y][target.x].color)
			return true;
		return false;
	}
	throw "エッ";
}

bool Field::forward(const OptAction& a0,
					const OptAction& a1,
					const OptAction& b0,
					const OptAction& b1) {
	if (m_resTurn == 0) return false;

	const OptAction* v[4] = {&a0, &a1, &b0, &b1 };
	Point pos[4];
	std::optional<Point> target[4];
	for (int i = 0; i < 4; i++) {
		pos[i] = m_agent[i];
		if (!*v[i]) continue; // 停留
		const Action &a = v[i]->value();
		if (!checkValid((AgentId)i, a)) continue; // 不正
		Point p = m_agent[i] + Neighbour8(a.dir);
		if (a.type == ActionType::Move)
			pos[i] = p;
		else if (a.type == ActionType::Remove) {
			target[i] = p;
		}
		else throw ("エッ");
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < i; j++) {
			// 行き先が被ったので無効
			if (pos[j] == pos[i]) {
				pos[i] = m_agent[i];
				pos[j] = m_agent[j];
			}
			// 除去先が被ったので無効
			if ((target[i] && target[j]) && target[j].value() == target[i].value()) {
				target[i].reset();
				target[j].reset();
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < i; j++) {
			// 念の為もう一回やっておく
			if (pos[j] == pos[i]) {
				pos[i] = m_agent[i];
				pos[j] = m_agent[j];
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < i; j++) {
			// 念の為更にもう一回やっておく（連鎖的に無効になるケースを考慮）
			if (pos[j] == pos[i]) {
				pos[i] = m_agent[i];
				pos[j] = m_agent[j];
			}
		}
	}

	// 除去処理
	for (int i = 0; i < 4; i++) {
		if (!target[i]) continue;
		m_field[target[i].value().y][target[i].value().x].color.reset();
	}
	
	// 塗り絵処理
	for (int i = 0; i < 4; i++) {
		m_agent[i] = pos[i];
		m_field[pos[i].y][pos[i].x].color = playerOf((AgentId)i);
	}

	m_resTurn--;
	return true;
}

bool Field::checkAllValid(const OptAction& a0,
						  const OptAction& a1,
						  const OptAction& b0,
						  const OptAction& b1) const {
	const OptAction* v[4] = {&a0, &a1, &b0, &b1 };
	for (int i = 0; i < 4; i++) {
		if (!*v[i]) continue;
		if (!checkValid((AgentId)i, v[i]->value())) return false;
	}
	return true;
}

bool Field::isBad(AgentId agentId, const Action & a) const {
	Point next = m_agent[(int)agentId] + Neighbour8(a.dir);
	if (outOfField(next)) return true;
	if (auto &c = m_field[next.y][next.x].color) {
		if (c.value() == playerOf(agentId)) return true;
	}
	else if (m_field[next.y][next.x].score < 0) return true;
	return false;
}

bool Field::isBad(PlayerId playerId, const PlayerMove & m) const {
	AgentId i0 = (AgentId)((int)playerId*2);
	AgentId i1 = (AgentId)((int)playerId*2 + 1);
	if (!m.a0 || !m.a1) return true;
	if (isBad(i0, m.a0.value())) return true;
	if (isBad(i1, m.a1.value())) return true;
	return false;
}

bool Field::isBad(const PlayerMove & m0, const PlayerMove & m1) const {
	const OptAction* v[4] = {&m0.a0, &m0.a1, &m1.a0, &m1.a1 };
	for (int i = 0; i < 4; i++) {
		if (!*v[i]) return true;
		if (isBad((AgentId)i, v[i]->value())) return true;
	}
	return false;
}

bool Field::forward(const PlayerMove & m0, const PlayerMove & m1) {
	return forward(m0.a0, m0.a1, m1.a0, m1.a1);
}

bool Field::checkAllValid(const PlayerMove & m0, const PlayerMove & m1) const {
	return checkAllValid(m0.a0, m0.a1, m1.a0, m1.a1);
}

bool Field::checkAllValid(PlayerId playerId, const PlayerMove & m) const {
	AgentId i0 = (AgentId)((int)playerId*2);
	AgentId i1 = (AgentId)((int)playerId*2 + 1);
	auto trg = [&](AgentId aid, const OptAction &a) {
		Point trg = m_agent[(int)aid];
		if (a) trg += Neighbour8(a->dir);
		return trg;
	};
	if (trg(i0, m.a0) == trg(i1, m.a1)) //味方同士で何しとんねん
		return false;
	return (!m.a0 || checkValid(i0, m.a0.value())) && (!m.a1 || checkValid(i1, m.a1.value()));
}

int Field::po() const {
	int ret = 0;
	for (int i = 0; i < h(); i++) {
		for (int j = 0; j < w(); j++) {
			ret += std::abs(m_field[i][j].score);
		}
	}
	return ret;
}

double Field::value() const {
	std::pair<int, int> res = calcScore();
	double score = (res.first - res.second)/(double)po();
	score *= 10.0;
	return std::tanh(score);
}

boost::property_tree::ptree Field::toPTree() const {
	using namespace boost::property_tree;
	ptree res;

	res.put("resTurn", std::to_string(m_resTurn));
	res.put("h", std::to_string(m_h));
	res.put("w", std::to_string(m_w));
	{
		ptree score;
		for (int i = 0; i < m_h; i++) {
			ptree row;
			for (int j = 0; j < m_w; j++) {
				ptree unit;
				unit.put("", std::to_string(m_field[i][j].score));
				row.push_back(std::make_pair("", unit));
			}
			score.push_back(std::make_pair("", row));
		}
		res.add_child("score", score);
	}
	{
		ptree color;
		for (int i = 0; i < m_h; i++) {
			ptree row;
			for (int j = 0; j < m_w; j++) {
				std::optional<PlayerId> c = m_field[i][j].color;
				std::string s = (c ? ((c.value() == PlayerId::A) ? "1" : "2") : "0");
				ptree unit;
				unit.put("", s);
				row.push_back(std::make_pair("", unit));
			}
			color.push_back(std::make_pair("", row));
		}
		res.add_child("color", color);
	}
	{
		ptree pos;
		for (int i = 0; i < 4; i++) {
			ptree unit;
			unit.put("x", std::to_string(m_agent[i].x));
			unit.put("y", std::to_string(m_agent[i].y));
			pos.push_back(std::make_pair("", unit));
		}
		res.add_child("pos", pos);
	}
	return res;
}


Action::Action(ActionType type, Direction8 dir) : type(type), dir(dir) {}

IntAction Action::ToInt(const std::optional<Action>& a) {
	if (!a) return 0;
	ActionType type = a.value().type;
	Direction8 dir = a.value().dir;
	return (type == ActionType::Remove ? 8 : 0) + dir + 1;
}

std::optional<Action> Action::FromInt(IntAction i) {
	using Opt = std::optional<Action>;
	if (i == 0) return Opt();
	if (i < 9) return Opt(Action(ActionType::Move, (Direction8)(i - 1)));
	return Opt(Action(ActionType::Remove, (Direction8)(i - 9)));
}

PlayerMove::PlayerMove() : a0(), a1() {}

PlayerMove::PlayerMove(const OptAction & a0, const OptAction & a1) : a0(a0), a1(a1) {}

int PlayerMove::toInt() const {
	return Action::ToInt(a0)*17 + Action::ToInt(a1);
}

PlayerMove PlayerMove::FromInt(IntMove i){
	return PlayerMove( Action::FromInt(i / Action::IntCount()), Action::FromInt(i % Action::IntCount()) );
}


}