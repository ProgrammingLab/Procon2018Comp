#include "DnnClient.h"
#include <cstdio>

namespace Procon2018 {

namespace asio = boost::asio;
using asio::ip::tcp;

DnnClient::DnnClient(const std::string ip, short unsigned int port)
: m_ioService()
, m_socket(tcp::socket(m_ioService)) {

	while (true) {
		boost::system::error_code error;
		m_socket.connect(tcp::endpoint(asio::ip::address::from_string(ip), port), error);

		if (error) {
			printf("\rchallenging to connect...");
			fflush(stdout);
			continue;
		}
		std::cout << std::endl;
		break;
	}
}

DnnClient::~DnnClient() {}

double DnnClient::Evaluate(const Field & field, PolicyPair & res) {
	std::vector<PolicyPair> res_;
	std::vector<Field> fields{field};
	double v = Evaluate(fields, res_)[0];
	res = res_[0];
	return v;
}

std::vector<double> DnnClient::Evaluate(const std::vector<Field>& field, std::vector<PolicyPair>& res) {
	using namespace boost::property_tree;
	ptree sent;
	{
		ptree states;
		for (int i = 0; i < (int)field.size(); i++) {
			states.push_back(std::make_pair("", field[i].toPTree()));
		}
		sent.add_child("states", states);
	}
	sendJson(m_socket, sent);

	auto toDouble = [](const std::string& s) {
		double d;
		std::stringstream(s) >> d;
		return d;
	};
	std::vector<double> ret;
	{
		ptree received = receiveJson(m_socket);

		int n = (int)received.get_child("result").size();
		res.resize(n);
		ret.resize(n);
		int resultId = 0;
		for (auto result : received.get_child("result")) {
			ret[resultId] = (toDouble( result.second.get<std::string>("value") ));

			const ptree& policyPair = result.second.get_child("policy");
			int playerId = 0;
			for (auto policy : policyPair) {
				int moveId = 0;
				for (auto unit : policy.second) {
					res[resultId][playerId][moveId] = toDouble( unit.second.get<std::string>("") );
					moveId++;
				}
				playerId++;
			}
			resultId++;
		}
	}
	return ret;
}


}