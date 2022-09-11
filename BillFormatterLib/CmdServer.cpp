#include "pch.h"
#include "CmdServer.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <filesystem>
#include <vector>
#include <string>


using error_msgs_t = std::vector<std::string>;


// Forward declarations
std::array<std::string, 2> const& get_out_dirs();
bool process_document(size_t bc, const std::string& doc, error_msgs_t& conversion_errors);


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

            if (data.empty())
            {
                data.emplace_back("no_data");
            }

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

size_t CmdServer::get_records_count(billCycleSelect_t bc)
{
    auto file_number = 0u;
    auto const out_dir = get_out_dirs()[bc];
    for (auto const& entry : std::filesystem::directory_iterator(out_dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".pdf")
        {
            ++file_number;
        }
    }
    return file_number;
}

void CmdServer::get_times(billCycleSelect_t bc, size_t offset, size_t count, message_data_t& ret)
{
    auto file_number = 0u;
    auto const out_dir = get_out_dirs()[bc];
    for (auto const& entry : std::filesystem::directory_iterator(out_dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".pdf")
        {
            if (offset <= file_number && file_number < offset + count)
            {
                auto const ftime = entry.last_write_time();
                auto const utctime = std::chrono::clock_cast<std::chrono::utc_clock>(ftime);
                auto const cftime = std::chrono::system_clock::to_time_t(
                        std::chrono::utc_clock::to_sys(utctime));
                //std::ostringstream sout;
                //sout << std::put_time(std::localtime(&cftime));
                //sout << ftime;
                //ret.push_back(sout.str());

                std::array<char, 100> buffer;
#pragma warning(push)
#pragma warning(disable : 4996)
                if (std::strftime(buffer.data(), buffer.size(), "%FT%T%z",
                        std::localtime(&cftime)))
                {
                    ret.push_back(buffer.data());
                }
#pragma warning(pop)
            }
            ++file_number;
        }
    }
}

void CmdServer::get_records(billCycleSelect_t bc, size_t offset, size_t count, message_data_t& ret)
{
    if (document_table.size() < offset + count)
    {
        document_table.resize(offset + count);
    }

    auto file_number = 0u;
    auto const out_dir = get_out_dirs()[bc];
    for (auto const& entry : std::filesystem::directory_iterator(out_dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".pdf")
        {
            if (offset <= file_number && file_number < offset + count)
            {
                std::ostringstream sout;

                // ID
                sout << std::setfill('0') << std::setw(6) << file_number;

                // Time
                auto const ftime = entry.last_write_time();
                auto const utctime = std::chrono::clock_cast<std::chrono::utc_clock>(ftime);
                auto const cftime = std::chrono::system_clock::to_time_t(
                        std::chrono::utc_clock::to_sys(utctime));

                std::array<char, 100> buffer;
#pragma warning(push)
#pragma warning(disable : 4996)
                if (std::strftime(buffer.data(), buffer.size(), "%FT%T%z",
                        std::localtime(&cftime)))
                {
                    sout << ' ' << buffer.data();
                }
#pragma warning(pop)

                // Status
                // TODO Determine conversion status based on ...!?
                sout << ' ' << "Converted";

                // Bill
                sout << ' ' << entry.path().filename();
                ret.emplace_back(std::move(sout.str()));

                auto doc_path = entry.path().filename().generic_string();
                auto ext_pos = doc_path.rfind('.');
                auto ext_count = doc_path.size() - ext_pos;
                doc_path.replace( ext_pos, ext_count, "");
                document_table[file_number] = doc_path;
            }

            ++file_number;
        }
    }
}

bool CmdServer::convert_document(billCycleSelect_t bc, size_t index, message_data_t& ret)
{
    if (0 <= index && index < document_table.size())
    {
        error_msgs_t errors;
        auto success = process_document(bc, document_table[index], errors);
        std::for_each(errors.begin(), errors.end(), [&ret](auto const& err) { ret.emplace_back(err); });
        return success;
    }
    return false;
}
