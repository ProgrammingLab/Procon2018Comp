#pragma once
#define NO_S3D_USING
#include <Siv3D.hpp>
#include <optional>
#include <queue>


namespace Procon2018 {


// •ûŒü‚Æ®”‚Ì‘Î‰‚ÍDNN‚Ìo—ÍŒ`®‚É‡‚í‚¹‚Ä‚ ‚é
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


s3d::Point Neighbour8(Direction8 dir);


}