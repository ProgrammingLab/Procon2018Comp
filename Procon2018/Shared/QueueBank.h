#pragma once
#include "Util.h"
#include <mutex>

namespace Procon2018 {


class QueueBank {
private:
	static std::vector<SP<std::queue<int>>> Resources;
	static std::vector<bool> IsFree;
	static std::mutex Mtx;
public:
	static SP<std::queue<int>> Allocate();
	static void Release(SP<std::queue<int>> resource);
};


}