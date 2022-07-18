#pragma once
#include "Commands.h"
#include "framework.h"

class CmdClient
{
        static const auto PIPELINE = 10u;

        zmq::context_t* context = nullptr;
        zmq::socket_t dealer;

public:
        CmdClient() = default;
        CmdClient(zmq::context_t& ctx);
        bool request_receive(IReq& req, message_data_t& ret);
};

