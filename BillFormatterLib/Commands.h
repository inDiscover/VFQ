#pragma once

class CmdServer;
using message_data_t = std::vector<std::string>;

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
	ReqRecords() : offset(0), count(0) {}
	ReqRecords(size_t off, size_t cnt) : offset(off), count(cnt) {}
	inline static const char* CMD = "ReqRec";
	const char* cmd() override { return CMD; }
	void create_cmd(message_data_t& ret) override;
	void get_reply(message_data_t& ret) override;

private:
	size_t offset = 0;
	size_t count = 0;
};
