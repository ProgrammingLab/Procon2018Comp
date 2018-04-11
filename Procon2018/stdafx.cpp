#include "stdafx.h"

namespace Procon2018 {


	s3d::Point Neighbour8(Direction8 dir) {
		int dx[8] = {1, 1, 0, -1, -1, -1, 0, 1};
		int dy[8] = {0, -1, -1, -1, 0, 1, 1, 1};
		return s3d::Point(dx[dir], dy[dir]);
	}


}