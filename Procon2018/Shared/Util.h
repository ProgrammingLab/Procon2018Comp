#pragma once

// Ubuntu環境において、これらをせずにboost::asioをincluideするとコンパイルエラー
// 環境構築が下手なだけかもしれないが
#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB
#define BOOST_ERROR_CODE_HEADER_ONLY
#define BOOST_SYSTEM_NO_LIB
#define BOOST_SYSTEM_NO_DEPRECATED

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/asio.hpp>

#include <optional>
#include <queue>
#include <cmath>
#include <memory>
#include <array>
#include <map>
#include <chrono>
#include <climits>
#include <iostream>


namespace Procon2018 {


template<class T> using SP = std::shared_ptr<T>;
template<class T> using UP = std::unique_ptr<T>;
template<class T> using WP = std::weak_ptr<T>;


struct Point {
	int x, y;
	Point();
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

	// [start, end)
	static int Next(int start, int end);

	// [0, count)
	static int Next(int count);

	// [0, 1]
	static double DNext();

	static int WeightRand(const std::vector<double> &weight);
};


}