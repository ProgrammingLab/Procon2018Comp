#pragma once


namespace Procon2018 {

//T: デフォルトコンストラクタが呼び出せること
template<class T, int MAX_SIZE>
class MockQueue {
private:

	T m_data[MAX_SIZE];

	int m_origin, m_size;

public:

	MockQueue() : m_data(), m_origin(0), m_size(0) {}

	int size() { return m_size; }

	void clear() { m_origin = m_size = 0; }

	void push(const T &item) {
		if (m_size == MAX_SIZE) throw "filled!";
		int index = m_origin + m_size;
		if (MAX_SIZE <= index) index -= MAX_SIZE;
		m_data[index] = item;
		m_size++;
	}

	T pop() {
		if (m_size == 0) throw "empty!";
		T& res = m_data[m_origin];
		m_origin++;
		m_size--;
		if (m_origin == MAX_SIZE) m_origin = 0;
		return res;
	}
};


}