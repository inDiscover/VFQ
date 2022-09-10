#pragma once

#include <vector>
#include <string>

class CmdServer;
using message_data_t = std::vector<std::string>;

using billCycleSelect_t = size_t;
struct billCycleSelect
{
    enum { bc1, bc2 };
};

class IRecordProvider
{
public:
        virtual size_t get_records_count(billCycleSelect_t bc) = 0;
        virtual void get_times(billCycleSelect_t bc, size_t offset, size_t count, message_data_t& ret) = 0;
        virtual void get_records(billCycleSelect_t bc, size_t offset, size_t count, message_data_t& ret) = 0;
        virtual bool convert_document(billCycleSelect_t bc, size_t index, message_data_t& ret) = 0;
};

class IReq
{
public:
        virtual const char* cmd() = 0;
        virtual void create_cmd(message_data_t&) = 0;
        virtual void get_reply(message_data_t&) = 0;
};

class ReqRecords : public IReq
{
        friend class CmdServer;

public:
        ReqRecords() : provider(nullptr), bill_cycle(billCycleSelect::bc1), offset(0), count(0) {}
        ReqRecords(billCycleSelect_t bc, size_t off, size_t cnt)
           : bill_cycle(bc), offset(off), count(cnt) {}
        inline static const char* CMD = "ReqRec";
        const char* cmd() override { return CMD; };
        void create_cmd(message_data_t& ret) override;
        void get_reply(message_data_t& ret) override;

private:
        IRecordProvider* provider = nullptr;
        billCycleSelect_t bill_cycle = billCycleSelect::bc1;
        size_t offset = 0;
        size_t count = 0;
};

class ReqCount : public IReq
{
        friend class CmdServer;

public:
        ReqCount() : provider(nullptr), bill_cycle(billCycleSelect::bc1) {}
        ReqCount(billCycleSelect_t bc) : bill_cycle(bc) {}
        inline static const char* CMD = "ReqCnt";
        const char* cmd() override { return CMD; };
        void create_cmd(message_data_t& ret) override;
        void get_reply(message_data_t& ret) override;

private:
        IRecordProvider* provider = nullptr;
        billCycleSelect_t bill_cycle = billCycleSelect::bc1;
};

class ReqConvertOne : public IReq
{
    friend class CmdServer;

public:
        ReqConvertOne() : provider(nullptr), bill_cycle(billCycleSelect::bc1), index(0) {}
        ReqConvertOne(billCycleSelect_t bc, size_t idx)
            : bill_cycle(bc), index(idx) {}
        inline static const char* CMD = "ReqConvOne";
        const char* cmd() override { return CMD; };
        void create_cmd(message_data_t& ret) override;
        void get_reply(message_data_t& ret) override;

private:
        IRecordProvider* provider = nullptr;
        billCycleSelect_t bill_cycle = billCycleSelect::bc1;
        size_t index = 0;
};
