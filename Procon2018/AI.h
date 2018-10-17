#pragma once
#include "Shared/Util.h"
#include "Shared/Field.h"
#include <future>
#include <thread>

namespace Procon2018 {


class AI {
private:
	std::optional<std::future<PlayerMove>> m_result;
protected:
	virtual PlayerMove calcNextMove(std::optional<std::pair<PlayerMove, PlayerMove>> moves) = 0;
public:
	virtual void init(const Field &field, PlayerId playerId) = 0;
	virtual void forward(std::optional<std::pair<PlayerMove, PlayerMove>> moves) {
		m_result = std::async(std::launch::async, [&, moves](){
			return calcNextMove(moves);
		});
	}
	virtual std::optional<PlayerMove> getNextMove() {
		if (m_result && m_result->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			auto ret = m_result->get();
			m_result.reset();
			return ret;
		}
		return std::optional<PlayerMove>();
	}
};


}