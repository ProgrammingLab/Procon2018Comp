#pragma once
#include "Util.h"
#include "Field.h"
#include "Node.h"

namespace Procon2018 {


class DnnClient {
private:

	boost::asio::io_service m_ioService;

	boost::asio::ip::tcp::socket m_socket;

public:

	DnnClient(const std::string ip, short unsigned int port);

	~DnnClient();
	
	// return: プレイヤー0から見た盤面の良さvalue[-1, 1]
	double Evaluate(const Field &field, PolicyPair &res);

	// 複数盤面を一度に評価するバージョン
	std::vector<double> Evaluate(const std::vector<Field> &field, std::vector<PolicyPair> &res);
};


}