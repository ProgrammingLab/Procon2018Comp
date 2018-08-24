#pragma once
#include <optional>
#include <queue>
#include <cmath>
#include <memory>
#include <array>
#include <map>
#include <chrono>
#include <climits>


namespace Procon2018 {


template<class T> using SP = std::shared_ptr<T>;
template<class T> using UP = std::unique_ptr<T>;
template<class T> using WP = std::weak_ptr<T>;


struct Point {
	int x, y;
	Point() : x(0), y(0) {}
	constexpr Point(int x, int y) : x(x), y(y) {}
	bool operator == (const Point &p) const;
	Point operator + (const Point &p) const;
	Point& operator += (const Point &p);
};


// 方向と整数の対応はDNNの出力形式に合わせてある
enum Direction8 {
	RU = 0, // (1, 1)
	U  = 1, // (0, 1)
	LU = 2, // (-1, 1)
	L  = 3, // (-1, 0)
	LD = 4, // (-1, -1)
	D  = 5, // (0, -1)
	RD = 6, // (1, -1)
	R  = 0, // (1, 0)
};

Point Neighbour8(Direction8 dir);

class Rand
{
private:
	static unsigned int x;
	static unsigned int y;
	static unsigned int z;
	static unsigned int w;

public:
	void InitializeWithTime();
	//[min, max)
	static const int Next(int min, int max);
	//[0, max)
	static int Next(int max);
	static const double DNext();
};


}