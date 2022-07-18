#include "pch.h"
#include "Commands.h"


void ReqRecords::create_cmd(message_data_t& ret)
{
    ret.emplace_back(CMD);
    ret.emplace_back(std::to_string(offset));
    ret.emplace_back(std::to_string(count));
}

void ReqRecords::get_reply(message_data_t& ret)
{
    std::vector<std::string> time_stamps;
    provider->get_times(offset, count, time_stamps);

    for (auto i = 0u; i < this->count; ++i)
    {
        std::ostringstream sout;
        std::time_t t = std::time(nullptr);
#pragma warning(push)
#pragma warning(disable : 4996)
        std::tm tm = *std::gmtime(&t);
#pragma warning(pop)

        sout << std::setw(4) << std::setfill('0') << i;
        //sout << ' ' << std::put_time(&tm, "%F_%T");
        sout << ' ' << time_stamps[i];
        sout << ' ' << "Converted";
        sout << ' ' << "Bill #" << i;
        ret.emplace_back(std::move(sout.str()));
    }
}
