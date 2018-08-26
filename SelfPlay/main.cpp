#include "../Procon2018/Shared/Util.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/asio.hpp>

#include "../Procon2018/Shared/Field.h"
#include "../Procon2018/Shared/DnnClient.h"

int main()
{
	// /*
	using namespace Procon2018;
	Field field = Field::RandomState();
	SP<DnnClient> dnn;
	while (true) {
		try {
			dnn = SP<DnnClient>(new DnnClient("127.0.0.1", 54215));
		}
		catch (...) {
			std::cout << "dnn initialize failed" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		break;
	}
	std::cout << "initialized dnn!" << std::endl;
	PolicyPair res;
	double v = dnn->Evaluate(field, res);
	std::cout << v << std::endl;
	for (int i = 0; i < res.size(); i++) {
		for (int j = 0; j < res[i].size(); j++) {
			std::cout << res[i][j] << " ";
		}
		std::cout << std::endl;
	}
	// */


	/*
	using namespace Procon2018;
	Field field = Field::RandomState();
	using namespace boost::property_tree;
	ptree pt = field.toPTree();
	std::stringstream ss;
	write_json(ss, pt, true);
	std::cout << ss.str();
	// */

	/*
	namespace asio = boost::asio;
	using asio::ip::tcp;

	asio::io_service io_service;
	tcp::socket socket(io_service);

	boost::system::error_code error;
	socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 54215), error);

	if (error) {
		std::cout << "connect failed : " << error.message() << std::endl;
	}
	else {
		std::cout << "connected" << std::endl;
	}

	using namespace boost::property_tree;
	ptree pt;
	pt.put("Ho.Ge", "hoge");
	std::stringstream ss;
	write_json(ss, pt);
	std::string body = ss.str();
	std::string sign = std::to_string(body.size()*sizeof(char));
	if (sign.size() > 10) throw "too large";
	while (sign.size() < 10) sign.push_back(' ');
	asio::write(socket, asio::buffer(sign)); //データの長さの10バイト文字列表現
	asio::write(socket, asio::buffer(body)); //自動で全部送るらしい
	// */

	/*
	using namespace boost::property_tree;
	ptree pt;
	pt.put("Ho.Ge", u8"ほげ");
	write_json("out.json", pt);
	// */
    return 0;
}