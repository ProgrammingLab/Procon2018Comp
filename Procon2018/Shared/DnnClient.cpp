#include "DnnClient.h"

namespace Procon2018 {

namespace asio = boost::asio;
using asio::ip::tcp;

DnnClient::DnnClient(const std::string ip, short unsigned int port)
: m_ioService()
, m_socket(tcp::socket(m_ioService)) {

	boost::system::error_code error;
	m_socket.connect(tcp::endpoint(asio::ip::address::from_string(ip), port), error);

	if (error) {
		throw "connect faild";
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
	{
		std::stringstream ss;
		write_json(ss, sent);
		std::string body = ss.str();
		std::string sign = std::to_string(body.size()*sizeof(char));
		if (sign.size() > 10) throw "too large";
		while (sign.size() < 10) sign.push_back(' ');
		asio::write(m_socket, asio::buffer(sign)); //データの長さの10バイト文字列表現
		asio::write(m_socket, asio::buffer(body)); //自動で全部送るらしい
	}
	auto toInt = [](const std::string& s) {
		int i;
		std::stringstream(s) >> i;
		return i;
	};
	auto toDouble = [](const std::string& s) {
		double d;
		std::stringstream(s) >> d;
		return d;
	};
	std::vector<double> ret;
	{
		asio::streambuf signBuffer;
		asio::read(m_socket, signBuffer, asio::transfer_exactly(10));
		int size = toInt( asio::buffer_cast<const char*>(signBuffer.data()) );
		asio::streambuf bodyBuffer;
		asio::read(m_socket, bodyBuffer, asio::transfer_exactly(size));
		std::stringstream jsonData(asio::buffer_cast<const char*>(bodyBuffer.data()));
		ptree received;
		read_json(jsonData, received);

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