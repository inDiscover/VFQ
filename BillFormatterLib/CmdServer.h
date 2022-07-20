#pragma once
#include "framework.h"
#include "Commands.h"
#include <string>
#include <variant>
#include <optional>
#include <iostream>
#include <vector>
#include <exception>
#include <stdexcept>


using req_t = std::variant<ReqRecords, ReqCount>;
using req_return_t = std::optional<req_t>;


class CmdServer : public IRecordProvider
{
        zmq::context_t& context;
        zmq::socket_t router;

public:
        CmdServer(zmq::context_t& ctx);
        bool receive_reply();

public:
        size_t get_records_count() override;
        void get_times(size_t offset, size_t count, message_data_t& ret) override;
        void get_records(size_t offset, size_t count, message_data_t& ret) override;

private:
        template<class ItBegin, class ItEnd>
                req_return_t deserialize(ItBegin it, const ItEnd end);
};


inline uint32_t parse_ulong(const std::string& text)
{
        char* processed = 0;
        auto ret = std::strtoul(text.data(), &processed, 0);
        if (text.data() == processed)
        {
                throw std::invalid_argument(std::string("Invalid argument '") + text + "'");
        }
        return ret;
}


template<class ItBegin, class ItEnd>
inline req_return_t CmdServer::deserialize(ItBegin it, const ItEnd end)
{
        const auto cmd = it->to_string();
        try
        {
                if (cmd == ReqRecords::CMD)
                {
                        if (std::distance(it, end) < 2)
                        {
                                throw std::length_error("Not enough data for '" + cmd + "'");
                        }
                        ReqRecords req{};
                        req.provider = this;
                        req.offset = parse_ulong((++it)->to_string());
                        req.count = parse_ulong((++it)->to_string());
                        return req;
                }
                else if (cmd == ReqCount::CMD)
                {
                        ReqCount req{};
                        req.provider = this;
                        return req;
                }
        }
        catch (const std::exception& err)
        {
                std::cerr << "Server failed to deserialize command " << cmd << ": " << err.what() << std::endl;
                return {};
        }

        std::cerr << "Server failed to deserialize unknown command " << cmd << std::endl;
        return {};
}
