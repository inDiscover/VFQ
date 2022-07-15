#include "pch.h"
#include "CmdServer.h"
#include <iterator>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <thread>


static int thread_main(CmdServer& server)
{
	for (;;)
	{
		server.receive_reply();
	}
	return 0;
}


CmdServer::CmdServer(zmq::context_t& ctx)
	: context(ctx)
{
	router = zmq::socket_t(context, zmq::socket_type::router);
	router.set(zmq::sockopt::rcvhwm, PIPELINE * 2);
	router.bind("tcp://*:8123");

	auto frontend_worker = std::thread([this] { thread_main(*this); });
	frontend_worker.detach();
}

bool CmdServer::receive_reply()
{
	std::vector<zmq::message_t> recv_msgs;
	std::string identifier;

	try
	{
		auto m_count = zmq::recv_multipart(router, std::back_inserter(recv_msgs));
		if (!m_count || 0 == *m_count)
		{
			std::cerr << "Server received no messages on router socket" << std::endl;
			return false;
		}
		std::cout << "Server got " << *m_count << " messages as a request" << std::endl;
	}
	catch (const std::exception& err)
	{
		std::cerr << "Server failed to receive from router socket: " << err.what() << std::endl;
		return false;
	}

	try
	{
		auto it = recv_msgs.begin();
		identifier = it->to_string();

		if (auto vreq = deserialize(++it, recv_msgs.end()))
		{
			message_data_t data;
			std::visit([&](auto&& request) { request.get_reply(data); }, *vreq);

			std::vector<zmq::mutable_buffer> send_msgs;
			send_msgs.emplace_back(zmq::buffer(identifier));
			std::for_each(data.begin(), data.end(), [&](auto& el) { send_msgs.emplace_back(zmq::buffer(el)); });
			zmq::send_multipart(router, send_msgs);
			std::cout << "Server replied " << send_msgs.size() << " messages" << std::endl;
		}
	}
	catch (const std::exception& err)
	{
		std::cerr << "Server failed to reply via router socket: " << err.what() << std::endl;
		return false;
	}

	return true;
}
