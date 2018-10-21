#pragma once
#include "Util.h"


namespace Procon2018 {


//T: デフォルトコンストラクタ, clear()が呼び出せること
template <class T>
class Bank {
private:
	static std::vector<SP<T>> Resources;

	static std::vector<bool> IsFree;

	static std::mutex Mtx;

public:

	static SP<T> Allocate() {
		for (int i = 0; i < Resources.size(); i++) {
			std::lock_guard<std::mutex> lock(Mtx);
			if (IsFree[i]) {
				IsFree[i] = false;
				return Resources[i];
			}
		}
		Resources.push_back(SP<T>(new T()));
		IsFree.push_back(false);
		return Resources.back();
	}

	static void Release(SP<T> resource) {
		resource->clear();
		for (int i = 0; i < Resources.size(); i++) {
			if (resource == Resources[i]) {
				std::lock_guard<std::mutex> lock(Mtx);
				IsFree[i] = true;
				return;
			}
		}
	}
};

template<class T>
std::vector<SP<T>> Bank<T>::Resources;

template<class T>
std::vector<bool> Bank<T>::IsFree;

template<class T>
std::mutex Bank<T>::Mtx;


}