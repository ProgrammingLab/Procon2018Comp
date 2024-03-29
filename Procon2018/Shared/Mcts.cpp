﻿#include "Mcts.h"


namespace Procon2018 {


Mcts::Mcts(const Field & rootState, SP<DnnClient> dnn) : m_rootState(rootState) {
	PolicyPair policyPair;
	dnn->Evaluate(m_rootState, policyPair);
	m_root = SP<Node>(new Node(policyPair));
}

Mcts::~Mcts() {}

Field Mcts::copyRootState() const {
	return Field(m_rootState);
}

SP<Node> Mcts::root() const {
	return m_root;
}

bool Mcts::isEnd() const {
	return m_rootState.isEnd();
}

void Mcts::addNoise() {
	if (!m_root) return;
	for (int i = 0; i < 2; i++) {
		IntMove j = Rand::Next(PlayerMove::IntCount());
		m_root->m_policyPair[i][j] += 1.0;
	}
}

bool Mcts::goDown(SP<Node> node, Field &field, std::vector<IntMoves>& path, bool kiresou) {
	if (node == nullptr) return false;

	IntMoves moveId = node->decideMoves(field, kiresou);
	auto movePair = Node::ToMoves(moveId);
	field.forward(movePair.first, movePair.second);
	path.push_back(moveId);

	// ゲーム終了
	if (field.resTurn() == 0) return true;
	
	auto next = node->m_next.find(moveId);
	if (next == node->m_next.end()) return false;

	return goDown(next->second, field, path, kiresou);
}

bool Mcts::goDown(Field & field, std::vector<IntMoves>& path, bool kiresou) {
	return goDown(m_root, field, path, kiresou);
}

void Mcts::backupWithExpansion(const std::vector<IntMoves>& path, double v, const PolicyPair & policyPair) {
	SP<Node> node = m_root;
	for (int i = 0; i < (int)path.size() - 1; i++) {
		node->backup(path[i], v);
		node = node->m_next[path[i]];
	}
	node->backup(path.back(), v);
	node->m_next[path.back()] = SP<Node>(new Node(policyPair));
}

void Mcts::backup(const std::vector<IntMoves>& path, double v) {
	SP<Node> node = m_root;
	for (int i = 0; i < (int)path.size() - 1; i++) {
		node->backup(path[i], v);
		node = node->m_next[path[i]];
	}
	node->backup(path.back(), v);
}

bool Mcts::selfNext(double temperature, SP<DnnClient> dnn) {
	if (m_root == nullptr) return false;

	PlayerMove moves[2];
	if (temperature < 1e-3) { // 単に訪問回数最大の手を選ぶ
		for (int i = 0; i < 2; i++) {
			int max = 0;
			IntMove maxIdx = -1;
			for (IntMove j = 0; j < PlayerMove::IntCount(); j++) {
				if (maxIdx == -1 || max < m_root->m_count[i][j]) {
					max = m_root->m_count[i][j];
					maxIdx = j;
				}
			}
			moves[i] = PlayerMove::FromInt(maxIdx);
		}
	}
	else {
		for (int i = 0; i < 2; i++) {
			std::vector<double> p(PlayerMove::IntCount());
			for (IntMove j = 0; j < PlayerMove::IntCount(); j++) {
				p[j] = std::pow(m_root->m_count[i][j], 1/temperature);
			}
			moves[i] = PlayerMove::FromInt(Rand::WeightRand(p));
		}
	}
	auto movePair = std::make_pair(moves[0], moves[1]);
	IntMoves intMoves = Node::ToInt(movePair);
	m_rootState.forward(movePair.first, movePair.second);

	if (m_rootState.isEnd()) { // ゲーム終了
		m_root = nullptr;
		return true;
	}

	auto itr = m_root->m_next.find(intMoves);
	if (itr == m_root->m_next.end()) { // 移動先ノードがまだ実体化されていなかった(DUCTなので起こりうる)
		PolicyPair pp;
		dnn->Evaluate(m_rootState, pp);
		m_root = SP<Node>(new Node(pp));
	}
	else {
		m_root = itr->second;
	}
	return false;
}


void Mcts::next(PlayerMove m1, SP<DnnClient> dnn) {
	PlayerMove m0;
	int max = 0;
	IntMove maxIdx = -1;
	for (IntMove j = 0; j < PlayerMove::IntCount(); j++) {
		if (maxIdx == -1 || max < m_root->m_count[0][j]) {
			max = m_root->m_count[0][j];
			maxIdx = j;
		}
	}
	m0 = PlayerMove::FromInt(maxIdx);
	auto movePair = std::make_pair(m0, m1);
	IntMoves intMoves = Node::ToInt(movePair);
	m_rootState.forward(movePair.first, movePair.second);

	if (m_rootState.isEnd()) { // ゲーム終了
		m_root = nullptr;
		return;
	}

	auto itr = m_root->m_next.find(intMoves);
	if (itr == m_root->m_next.end()) { // 移動先ノードがまだ実体化されていなかった(DUCTなので起こりうる)
		PolicyPair pp;
		double v = dnn->Evaluate(m_rootState, pp);
		m_root = SP<Node>(new Node(pp));
	}
	else {
		m_root = itr->second;
	}
}


}