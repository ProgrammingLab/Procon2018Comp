#include "Util.h"

namespace Procon2018 {


Point Neighbour8(Direction8 dir) {
	int dx[8] = {1, 0, -1, -1, -1, 0, 1, 1};
	int dy[8] = {1, 1, 1, 0, -1, -1, -1, 0};
	return Point(dx[dir], dy[dir]);
}


unsigned int Rand::x = 123456789;
unsigned int Rand::y = 362436069;
unsigned int Rand::z = 521288629;
unsigned int Rand::w = 993251954;


void Rand::InitializeWithTime() {
	using namespace std::chrono;
	milliseconds t = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	//cout << t.count() << " ms" << endl;
	w = t.count() % UINT_MAX;
	//cout << w << endl;
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