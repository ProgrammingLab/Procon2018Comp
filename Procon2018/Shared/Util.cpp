#include "Util.h"

namespace Procon2018 {


namespace asio = boost::asio;
using asio::ip::tcp;
using namespace boost::property_tree;


Point Neighbour8(Direction8 dir) {
	int dx[8] = {1, 0, -1, -1, -1, 0, 1, 1};
	int dy[8] = {1, 1, 1, 0, -1, -1, -1, 0};
	return Point(dx[dir], dy[dir]);
}

int toInt(const std::string & s) {
	int i;
	std::stringstream(s) >> i;
	return i;
}

void sendJson(boost::asio::ip::tcp::socket & socket, const boost::property_tree::ptree & sent) {
	std::stringstream ss;
	write_json(ss, sent);
	std::string body = ss.str();
	std::string sign = std::to_string(body.size()*sizeof(char));
	if (sign.size() > 10) throw "too large";
	while (sign.size() < 10) sign.push_back(' ');
	asio::write(socket, asio::buffer(sign)); //データの長さの10バイト文字列表現
	asio::write(socket, asio::buffer(body)); //自動で全部送るらしい
}

boost::property_tree::ptree receiveJson(boost::asio::ip::tcp::socket & socket) {
	asio::streambuf signBuffer;
	asio::read(socket, signBuffer, asio::transfer_exactly(10));
	int size = toInt( asio::buffer_cast<const char*>(signBuffer.data()) );
	asio::streambuf bodyBuffer;
	asio::read(socket, bodyBuffer, asio::transfer_exactly(size));
	std::string bodyStr = std::string(asio::buffer_cast<const char*>(bodyBuffer.data()));
	bodyStr.resize(size); // ごみデータが入ることがあるので切り詰める
	std::stringstream jsonData(bodyStr);
	ptree received;
	read_json(jsonData, received);
	return std::move(received);
}


unsigned int Rand::x = 123456789;
unsigned int Rand::y = 362436069;
unsigned int Rand::z = 521288629;
unsigned int Rand::w = 993251954;


void Rand::InitializeWithTime() {
	using namespace std::chrono;
	milliseconds t = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	//cout << t.count() << " ms" << endl;
	unsigned seed = t.count() % UINT_MAX;
	std::cout << "rand seed: " << seed << std::endl;
	Initialize(seed);
}

void Rand::Initialize(unsigned int seed) {
	w = seed;
	for (int i = 0; i < 20; i++) {
		Next(1);
	}
}

int Rand::Next(int start, int end) {
	unsigned int t;
	t = x ^ (x << 11);
	x = y;
	y = z;
	z = w;
	w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
	return (int)(start + (double)w*(end - start) / ((double)UINT_MAX + 1));
}

int Rand::Next(int count) {
	return Next(0, count);
}

double Rand::DNext() {
	return Next(INT_MAX)/(double)(INT_MAX - 1);
}

int Rand::WeightRand(const std::vector<double>& weight) {
	double sum = 0;
	double maxW = 0;
	int maxIdx = -1;
	for (int i = 0; i < (int)weight.size(); i++) {
		sum += weight[i];
		if (maxIdx == -1 || maxW < weight[i]) {
			maxW = weight[i];
			maxIdx = i;
		}
	}
	double r = DNext();
	for (int i = 0; i < (int)weight.size(); i++) {
		r -= weight[i]/sum;
		if (r <= 1e-10) return i;
	}
	return maxIdx;
}

Point::Point() : x(0), y(0) {}

bool Point::operator==(const Point & p) const {
	return x == p.x && y == p.y;
}

Point Point::operator+(const Point &p) const {
	return Point(x + p.x, y + p.y);
}

Point& Point::operator+=(const Point &p) {
	x += p.x;
	y += p.y;
	return *this;
}


}