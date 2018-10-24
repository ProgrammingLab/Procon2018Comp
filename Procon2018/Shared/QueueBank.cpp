#include "QueueBank.h"

namespace Procon2018 {


std::vector<SP<std::queue<int>>> QueueBank::Resources;
std::vector<bool> QueueBank::IsFree;
std::mutex QueueBank::Mtx;

SP<std::queue<int>> QueueBank::Allocate() {
	for (int i = 0; i < Resources.size(); i++) {
		std::lock_guard<std::mutex> lock(Mtx);
		if (IsFree[i]) {
			IsFree[i] = false;
			return Resources[i];
		}
	}
	Resources.push_back(SP<std::queue<int>>(new std::queue<int>()));
	IsFree.push_back(false);
	return Resources.back();
}

void QueueBank::Release(SP<std::queue<int>> resource) {
	while (!resource->empty()) resource->pop();
	for (int i = 0; i < Resources.size(); i++) {
		if (resource == Resources[i]) {
			std::lock_guard<std::mutex> lock(Mtx);
			IsFree[i] = true;
			return;
		}
	}
}


}