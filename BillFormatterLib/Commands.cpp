#include "pch.h"
#include "Commands.h"
#include <string>
#include <iostream>


void ReqRecords::create_cmd(message_data_t& ret)
{
    ret.emplace_back(CMD);
    ret.emplace_back(std::to_string(bill_cycle));
    ret.emplace_back(std::to_string(offset));
    ret.emplace_back(std::to_string(count));
}

void ReqRecords::get_reply(message_data_t& ret)
{
    provider->get_records(bill_cycle, offset, count, ret);
}


void ReqCount::create_cmd(message_data_t& ret)
{
    ret.emplace_back(CMD);
    ret.emplace_back(std::to_string(bill_cycle));
}

void ReqCount::get_reply(message_data_t& ret)
{
    auto const rec_count = provider->get_records_count(bill_cycle);
    ret.emplace_back(std::to_string(rec_count));
    //std::cout << "ReqCount: Counted " << rec_count << " records" << std::endl;
}


void ReqConvertOne::create_cmd(message_data_t& ret)
{
    ret.emplace_back(CMD);
    ret.emplace_back(std::to_string(bill_cycle));
    ret.emplace_back(std::to_string(index));
}

void ReqConvertOne::get_reply(message_data_t& ret)
{
    // Convert this one document
    if (provider->convert_document(bill_cycle, index, ret))
    {
        // Return the converted document's record
        provider->get_records(bill_cycle, index, 1, ret);
    }
}


