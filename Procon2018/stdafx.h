#pragma once
#define NO_S3D_USING
#include <Siv3D.hpp>
#include <optional>


namespace Procon2018 {


enum Direction8 {
	R = 0,  // (1, 0)
	RU = 1, // (1, 1)
	U = 2,  // (0, 1)
	LU = 3, // (-1, 1)
	L = 4,  // (-1, 0)
	LD = 5, // (-1, -1)
	D = 6,  // (0, -1)
	RD = 7, // (1, -1)
};


s3d::Point Neighbour8(Direction8 dir);


}