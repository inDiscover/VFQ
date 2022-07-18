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
    provider->get_records(offset, count, ret);
}
