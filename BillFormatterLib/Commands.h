#pragma once

#include <vector>
#include <string>

class CmdServer;
using message_data_t = std::vector<std::string>;

class IRecordProvider
{
public:
        virtual size_t get_records_count() = 0;
        virtual void get_times(size_t offset, size_t count, message_data_t& ret) = 0;
        virtual void get_records(size_t offset, size_t count, message_data_t& ret) = 0;
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
        ReqRecords() : provider(nullptr), offset(0), count(0) {}
        ReqRecords(IRecordProvider* srv, size_t off, size_t cnt) : provider(srv), offset(off), count(cnt) {}
        inline static const char* CMD = "ReqRec";
        const char* cmd() override { return CMD; };
        void create_cmd(message_data_t& ret) override;
        void get_reply(message_data_t& ret) override;

private:
        IRecordProvider* provider = nullptr;
        size_t offset = 0;
        size_t count = 0;
};

class ReqCount : public IReq
{
        friend class CmdServer;

public:
        ReqCount() : provider(nullptr) {}
        ReqCount(IRecordProvider* srv) : provider(srv) {}
        inline static const char* CMD = "ReqCnt";
        const char* cmd() override { return CMD; };
        void create_cmd(message_data_t& ret) override;
        void get_reply(message_data_t& ret) override;

private:
        IRecordProvider* provider = nullptr;
};
