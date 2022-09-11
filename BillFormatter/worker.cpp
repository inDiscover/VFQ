#include "worker.h"
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

namespace
{
    void create_child_process(const char* cmd_line, PROCESS_INFORMATION* proc_info)
    {
#if _MSC_VER >= 1910
        STARTUPINFOA start_info;
        BOOL success = FALSE;

        // Set up members of the PROCESS_INFORMATION structure.

        ZeroMemory(proc_info, sizeof(PROCESS_INFORMATION));

        // Set up members of the STARTUPINFO structure.
        // This structure specifies the STDIN and STDOUT handles for redirection.

        ZeroMemory(&start_info, sizeof(STARTUPINFO));
        start_info.cb = sizeof(STARTUPINFO);
        //start_info.hStdError = g_hChildStd_OUT_Wr;
        //start_info.hStdOutput = g_hChildStd_OUT_Wr;
        //start_info.hStdInput = g_hChildStd_IN_Rd;
        //start_info.dwFlags |= STARTF_USESTDHANDLES;

        // Create the child process.

        success = CreateProcessA(NULL,
                (LPSTR)cmd_line,// command line
                NULL,          // process security attributes
                NULL,          // primary thread security attributes
                //TRUE,          // handles are inherited
                FALSE,          // handles are inherited
                0,             // creation flags
                NULL,          // use parent's environment
                NULL,          // use parent's current directory
                &start_info,   // STARTUPINFO pointer
                proc_info);    // receives PROCESS_INFORMATION

        // If an error occurs, exit the application.
        if (!success)
        {
            std::array<char, 500> buffer;
            ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, ::GetLastError(), 0, buffer.data(), (DWORD)buffer.size(), nullptr);
            std::cerr << "Failed to spawn child process: " << buffer.data() << std::endl;
        }
        else
        {
            // Close handles to the child process and its primary thread.
            // Some applications might keep these handles to monitor the status
            // of the child process, for example.

            //CloseHandle(proc_info->hProcess);
            //CloseHandle(proc_info->hThread);

            // Close handles to the stdin and stdout pipes no longer needed by the child process.
            // If they are not explicitly closed, there is no way to recognize that the child process has ended.

            //CloseHandle(g_hChildStd_OUT_Wr);
            //CloseHandle(g_hChildStd_IN_Rd);
        }
#endif
    }
}

worker::worker()
    : proc_info(nullptr)
{
}

worker::worker(const worker& other)
    : proc_info(other.proc_info)
{
    const_cast<worker&>(other).proc_info = nullptr;
}

worker::worker(worker&& other) noexcept
: proc_info(std::exchange(other.proc_info, nullptr))
{
}

worker::worker(const std::string& prog_name, int backend_port, const std::array<std::string, 2>& out_dirs, size_t bc, const std::vector<std::string>& docs)
{
    auto cmd_line = prog_name + " -worker " + " -backend=" + std::to_string(backend_port) + " -billcycle=" + std::to_string(bc) +
        " -out1=" + out_dirs[0] + " " + " -out2=" + out_dirs[1] + " ";
    std::for_each(docs.begin(), docs.end(), [&cmd_line](auto& el) { cmd_line += el + " "; });
    proc_info = new PROCESS_INFORMATION;
    create_child_process(cmd_line.data(), proc_info);
}

worker::~worker()
{
    auto exit_code = 0ul;
    if (proc_info)
    {
        auto success = GetExitCodeProcess(proc_info->hProcess, &exit_code);
        if (success && exit_code == STILL_ACTIVE)
        {
            CloseHandle(proc_info->hProcess);
            CloseHandle(proc_info->hThread);

            // Terminate child process forcefully
            ::TerminateProcess(proc_info->hProcess, 1);
        }
        delete proc_info;
    }
}

worker& worker::operator=(const worker& other)
{
    return *this = worker(other);
}

worker& worker::operator=(worker&& other) noexcept
{
    std::swap(proc_info, other.proc_info);
    return *this;
}
