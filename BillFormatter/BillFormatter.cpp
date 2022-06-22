// BillFormatter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <array>
#include <vector>
#include <deque>
#include <string>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>

#if _OPENMP
#	include <omp.h>
#endif

#include "wkhtmltox/pdf.h"

#include "HtmlConverter.h"

using namespace std;
using namespace std::chrono_literals;

deque<html_converter> converters;
std::vector<std::thread> g_threads;
bool g_terminate_pool = false;
std::mutex g_queue_mtx;
std::condition_variable g_cv_pool;
size_t g_done_count = 0u;
size_t g_job_count = 0u;
size_t g_shutdown_count = 0u;
size_t max_threads = 1;
std::mutex g_done_mtx;
std::condition_variable g_cv_done;
std::mutex g_shutdown_mtx;
std::condition_variable g_cv_shutdown;
const auto TIMEOUT_ALL_JOBS_FINISHED(5000ms);
const auto TIMEOUT_SHUTDOWN_WORKERS(5000ms);

namespace
{
    /* Print out loading progress information */
    void progress_changed(wkhtmltopdf_converter* c, int p) {
        printf("%3d%%\r", p);
        fflush(stdout);
    }

    /* Print loading phase information */
    void phase_changed(wkhtmltopdf_converter* c) {
        int phase = wkhtmltopdf_current_phase(c);
        printf("%s\n", wkhtmltopdf_phase_description(c, phase));
    }

    /* Print a message to stderr when an error occurs */
    void error(wkhtmltopdf_converter* c, const char* msg) {
        fprintf(stderr, "Error: %s\n", msg);
    }

    /* Print a message to stderr when a warning is issued */
    void warning(wkhtmltopdf_converter* c, const char* msg) {
        fprintf(stderr, "Warning: %s\n", msg);
    }
}

void print_usage()
{
    cout << "BillFormatter usage:\n";
    cout << "  BillFormatter.exe <Options> <Arguments>\n";
    cout << "  Options:\n";
    cout << "    -h: Print help\n";
    cout << "  Arguments:\n";
    cout << "    One or more documents to be converted to PDF\n";
    cout << endl;
}

bool wait_for_shutdown()
{
    std::unique_lock<std::mutex> lock(g_shutdown_mtx);
    return g_cv_shutdown.wait_for(lock, TIMEOUT_SHUTDOWN_WORKERS, [] { return g_shutdown_count == max_threads; });
}

void shutdown_pool()
{
    {
        std::unique_lock<std::mutex> lock(g_queue_mtx);
        g_terminate_pool = true; // use this flag in condition.wait
        std::cout << "Signal all threads to shutdown" << std::endl;
    }

    g_cv_pool.notify_all(); // wake up all threads.

    if (!wait_for_shutdown())
    {
        std::cerr << "Timeout occurred while waiting for thread pool to shutdown. Terminating ..." << std::endl;
        std::cerr.flush();
        std::abort();
    }

    // Join all threads.
    for (std::thread& th : g_threads)
    {
        if (th.joinable())
        {
            //th.join();
            th.detach();
        }
    }

    converters.clear();
}

bool wait_for_all_jobs()
{
    std::unique_lock<std::mutex> lock(g_done_mtx);
    return g_cv_done.wait_for(lock, TIMEOUT_ALL_JOBS_FINISHED, [] { return g_done_count == g_job_count; });
}

void signal_job_done()
{
    {
        std::unique_lock<std::mutex> lock(g_done_mtx);
        ++g_done_count;
    }
    g_cv_done.notify_all();
}

void add_job(const std::string& doc)
{
    std::cout << "Add job for " << doc << std::endl;

    {
        std::unique_lock<std::mutex> lock(g_queue_mtx);
        converters.emplace_back(doc);
        ++g_job_count;
    }

    g_cv_pool.notify_one();
}

static unsigned int __stdcall thread_main()
{
    for (;;)
    {
            std::unique_lock<std::mutex> lock(g_queue_mtx);
            g_cv_pool.wait(lock, [] {
                return !converters.empty() || g_terminate_pool;
            });

            if (g_terminate_pool)
            {
                std::cout << "Terminate pool thread" << std::endl;
                std::unique_lock<std::mutex> lock(g_shutdown_mtx);
                ++g_shutdown_count;
                g_cv_shutdown.notify_one();
                return 0;
            }
            auto job = converters.front();
            converters.pop_front();
            lock.unlock();

            std::cout << "Processing job for " << job.get_doc() << std::endl;
            if (!job.convert())
            {
                std::cerr << "Failed to convert " << job.get_doc() << ". Error=" << job.get_html_error_code() << std::endl;
            }
            else
            {
                const unsigned char* doc_buffer = nullptr;
                std::cout << "Converted document size " << job.get_output_buffer(&doc_buffer) << std::endl;
            }
            signal_job_done();
    }

    return 1;
}

int main(int argc, char* argv[])
{
    vector<string> inputDocs;

    wkhtmltopdf_init(1);
    cout << "WKHtmlToX version " << wkhtmltopdf_version() << endl;

    // Parsing the command line arguments and options
    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];

        //string opt;
        //transform(arg.begin(), arg.end(), back_inserter(opt),
        //    [](unsigned char c) { return tolower(c); });

        if (arg == "-h")
        {
            print_usage();
        }
        else
        {
            inputDocs.push_back(arg);
        }
    }

    max_threads = std::thread::hardware_concurrency();

#if _OPENMP
    max_threads = std::max(max_threads, omp_get_num_procs());
#endif
    max_threads = 1;

    // Print the list of arguments
    cout << "Input documents:\n";
    for (auto doc : inputDocs)
    {
        cout << doc << endl;

        add_job(doc);

        //auto out = doc + ".pdf";
        //auto* global_settings = wkhtmltopdf_create_global_settings();
        //wkhtmltopdf_set_global_setting(global_settings, "out", out.data());
        //auto* converter = wkhtmltopdf_create_converter(global_settings);
        //auto* settings = wkhtmltopdf_create_object_settings();
        //wkhtmltopdf_set_object_setting(settings, "page", doc.data());
        //wkhtmltopdf_add_object(converter, settings, nullptr);

        ///* Call the progress_changed function when progress changes */
        //wkhtmltopdf_set_progress_changed_callback(converter, progress_changed);

        ///* Call the phase _changed function when the phase changes */
        //wkhtmltopdf_set_phase_changed_callback(converter, phase_changed);

        ///* Call the error function when an error occurs */
        //wkhtmltopdf_set_error_callback(converter, error);

        ///* Call the warning function when a warning is issued */
        //wkhtmltopdf_set_warning_callback(converter, warning);
        //wkhtmltopdf_convert(converter);
    }
    cout << endl;

    //while (0 < converters.size())
    //{
    //        auto job = converters.front();
    //        converters.pop_front();

    //        std::cout << "Converting document " << job.get_doc() << std::endl;
    //        if (!job.convert())
    //        {
    //            std::cerr << "Failed to convert " << job.get_doc() << std::endl;
    //        }
    //}


    for (auto i = 0u; i < max_threads; ++i)
    {
        g_threads.emplace_back(thread_main);
    }

    //std::this_thread::sleep_for(2000ms);
    if (wait_for_all_jobs())
    {
        std::cout << "All " << g_job_count << " jobs have finished" << std::endl;
    }
    else
    {
        std::cerr << "Timeout occurred while waiting for jobs to finish" << std::endl;
    }

    shutdown_pool();
    wkhtmltopdf_deinit();

    return 0;
}

// Topics:
// -------
//
// / Review arrays
// / std::string
// / Command line parser: input00.html input01.html input02.html
// / Operator overloading
//
// - constexpr
// - struct
// - Using additional libs
// - Version control via Git
// - template
// / friend
// - CLR
// - virtual
// - public
// - explicit
// - DLL example project

// - <fstream>, <ofstream>: open, truncate
//   https://en.cppreference.com/w/cpp/io/basic_ofstream


