#define BOOST_DATE_TIME_NO_LIB
#define BOOST_REGEX_NO_LIB
#define BOOST_ERROR_CODE_HEADER_ONLY
#define BOOST_SYSTEM_NO_LIB

#include <cstdio>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/asio.hpp>

int main()
{
	// /*
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
	// */

	/*
	using namespace boost::property_tree;
	ptree pt;
	pt.put("Ho.Ge", u8"‚Ù‚°");
	write_json("out.json", pt);
	// */
    return 0;
}