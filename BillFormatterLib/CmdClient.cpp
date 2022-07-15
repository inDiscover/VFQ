#include "pch.h"
#include "CmdClient.h"
#include "framework.h"
#include <vector>
#include <iostream>

CmdClient::CmdClient()
{
}

CmdClient::CmdClient(zmq::context_t& ctx)
	: context(&ctx)
{
    dealer = zmq::socket_t(ctx, zmq::socket_type::dealer);
    dealer.connect("tcp://127.0.0.1:8123");
}

bool CmdClient::request_receive(IReq& req, message_data_t& ret)
{
	message_data_t data;
	req.create_cmd(data);

	std::vector<zmq::const_buffer> send_msgs;
	std::for_each(data.begin(), data.end(), [&](const auto& el) { send_msgs.emplace_back(zmq::buffer(el)); });

	try
	{
		auto sent_count = zmq::send_multipart(dealer, send_msgs);
		if (!sent_count)
		{
			std::cerr << "Client failed to send request" << std::endl;
			return false;
		}
	}
	catch (const std::exception& err)
	{
		std::cerr << "Client failed to send request '" << req.cmd() << "': " << err.what() << std::endl;
		return false;
	}

	try
	{
		std::vector<zmq::message_t> recv_msgs;
		const auto got_count = zmq::recv_multipart(dealer, std::back_inserter(recv_msgs));
		if (!got_count)
		{
			std::cerr << "Client failed to send request" << std::endl;
			return false;
		}

		std::for_each(recv_msgs.begin(), recv_msgs.end(), [&](const auto& el) { ret.push_back(el.to_string()); });
	}
	catch (const std::exception& err)
	{
		std::cerr << "Client failed to receive reply to '" << req.cmd() << "': " << err.what() << std::endl;
		return false;
	}

	return true;
}
