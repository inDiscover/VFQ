#pragma once
#include <string>
#include <vector>

#if _MSC_VER >= 1910
#   define WIN32_LEAN_AND_MEAN
#   include <Windows.h>
#   include <processthreadsapi.h>
#endif

class worker
{
public:
    worker();
    worker(const worker& other);
    worker(worker&& other) noexcept;
    worker(const std::string& prog_name, int backend_port, const std::string& out_dir, const std::vector<std::string>& docs);
    virtual ~worker();
    worker& operator=(const worker& other);
    worker& operator=(worker&& other) noexcept;

private:
    PROCESS_INFORMATION* proc_info = nullptr;
};

