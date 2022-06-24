// BillFormatter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iomanip>
#include <array>
#include <ostream>
#include <sstream>
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
#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "HtmlConverter.h"
#include "worker.h"

using namespace std;
using namespace std::chrono_literals;

bool g_is_worker = false;
std::vector<worker> g_workers;
deque<html_converter> converters;
bool g_terminate_pool = false;
//std::mutex g_queue_mtx;
//std::condition_variable g_cv_pool;
size_t g_done_count = 0u;
size_t g_job_count = 0u;
size_t g_shutdown_count = 0u;
size_t max_threads = 1;
//std::mutex g_done_mtx;
//std::condition_variable g_cv_done;
//std::mutex g_shutdown_mtx;
//std::condition_variable g_cv_shutdown;
//const auto TIMEOUT_ALL_JOBS_FINISHED(5000ms);
//const auto TIMEOUT_SHUTDOWN_WORKERS(5000ms);

static const std::string MSG_JOB_REQ = "JOB_REQ";
static const std::string MSG_WORKER_READY = "WORKER_READY";
static const std::string MSG_WORKER_TERM = "WORKER_TERM";

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
    //std::unique_lock<std::mutex> lock(g_shutdown_mtx);
    //return g_cv_shutdown.wait_for(lock, TIMEOUT_SHUTDOWN_WORKERS, [] { return g_shutdown_count == max_threads; });
    std::this_thread::sleep_for(5s);
    return true;
}

void signal_shutdown_pool(zmq::socket_t& broker)
{
    // Signal shutdown of all workers
    for (auto i=0; i<max_threads; ++i)
    {
        //  Next message gives us least recently used worker
        zmq::message_t identity_msg;
        auto rc = broker.recv(identity_msg, zmq::recv_flags::none);
        if (!rc)
        {
            std::cerr << "Failed to receive workers identity" << std::endl;
            break;
        }

        std::array<zmq::const_buffer, 3> req_msgs =
        {
            zmq::buffer(identity_msg.to_string()),
            zmq::str_buffer(""),          // envelop delimiter
            zmq::buffer(MSG_WORKER_TERM)  // shutdown request
        };
        try
        {
            if (!zmq::send_multipart(broker, req_msgs))
            {
                std::cerr << "Failed to send WORKER_TERM to worker " << identity_msg.to_string() << std::endl;
            }
            else
            {
                std::cout << "Sent WORKER_TERM to " << identity_msg.to_string() << std::endl;
            }
        }
        catch (const zmq::error_t& err)
        {
            std::cerr << "Failed to send WORKER_TERM to worker " << identity_msg.to_string() << ": " << err.what() << std::endl;
        }
    }
}

void shutdown_pool(zmq::socket_t& broker)
{
    {
        //std::unique_lock<std::mutex> lock(g_queue_mtx);
        g_terminate_pool = true;
        signal_shutdown_pool(broker);
        std::cout << "Signaled to shutdown worker" << std::endl;
    }

    //g_cv_pool.notify_all(); // wake up all threads.

    if (!wait_for_shutdown())
    {
        std::cerr << "Timeout occurred while waiting for thread pool to shutdown. Terminating ..." << std::endl;
        std::cerr.flush();
        std::abort();
    }

    //// Join all threads.
    //for (std::thread& th : g_threads)
    //{
    //    if (th.joinable())
    //    {
    //        //th.join();
    //        th.detach();
    //    }
    //}

    converters.clear();
}

bool wait_for_all_jobs()
{
    //std::unique_lock<std::mutex> lock(g_done_mtx);
    //return g_cv_done.wait_for(lock, TIMEOUT_ALL_JOBS_FINISHED, [] { return g_done_count == g_job_count; });
    std::this_thread::sleep_for(60s);
    return true;
}

void signal_worker_ready(zmq::socket_t& worker, const std::string& worker_id)
{
    {
        //std::unique_lock<std::mutex> lock(g_done_mtx);
        //++g_done_count;
    }
    //g_cv_done.notify_all();

    std::array<zmq::const_buffer, 2> req_msgs =
    {
        zmq::str_buffer(""),    // envelop delimiter
        zmq::buffer(MSG_WORKER_READY)
    };
    try
    {
        if (!zmq::send_multipart(worker, req_msgs))
        {
            std::cerr << "Worker " << worker_id << " failed to send WORKER_READY." << std::endl;
        }
    }
    catch (const zmq::error_t& err)
    {
        std::cerr << "Worker " << worker_id << " failed to send WORKER_READY." << err.what() << std::endl;
    }
}

void add_job(const std::string& doc)
{
    std::cout << "Add job for " << doc << std::endl;

    {
        //std::unique_lock<std::mutex> lock(g_queue_mtx);
        converters.emplace_back(doc);
        ++g_job_count;
    }

    //g_cv_pool.notify_one();
}

static unsigned int worker_main(zmq::context_t& context, int backend_port)
{
    zmq::socket_t worker(context, zmq::socket_type::dealer);
    std::ostringstream sout;
    sout << std::setw(5) << std::setfill('0') << std::hex << std::this_thread::get_id();
    auto worker_id = sout.str();
    std::cout << "Worker " << worker_id << " connects to port " << backend_port << std::endl;
    //worker.setsockopt(ZMQ_IDENTITY, worker_id.data(), worker_id.size());
    zmq_setsockopt(worker.handle(), ZMQ_IDENTITY, worker_id.data(), worker_id.size());
    auto endpoint = std::string("tcp://localhost:") + std::to_string(backend_port);
    worker.connect(endpoint.data());

    for (;;)
    {
        // Unlock this worker
        //std::unique_lock<std::mutex> lock(g_queue_mtx);
        //g_cv_pool.wait(lock, [] {
        //    return !converters.empty() || g_terminate_pool;
        //});

        std::cout << "Worker " << worker_id << " signals WORKER_READY" << std::endl;
        signal_worker_ready(worker, worker_id);

        std::cout << "Worker " << worker_id << " waiting for job requests..." << std::endl;

        std::vector<zmq::message_t> recv_msgs;
        const auto rc = zmq::recv_multipart(
            worker, std::back_inserter(recv_msgs));
        if (!rc)
        {
            std::cerr << "Worker " << worker_id << " failed to receive multi-part job request" << std::endl;
            break;
        }
        std::cout << "Got " << *rc
                  << " messages" << std::endl;
        zmq::message_t& identity_msg = recv_msgs[0];
        zmq::message_t& delimiter_msg = recv_msgs[1];
        zmq::message_t& workload_msg = recv_msgs[2];

        //zmq::message_t identity_msg;
        //auto rc = worker.recv(identity_msg, zmq::recv_flags::none); // worker identity
        //std::cout << "Worker " << worker_id << " received identity " << std::quoted(identity_msg.to_string()) << " [" << rc.value() << "]" << std::endl;
        //zmq::message_t workload_msg;
        //rc = worker.recv(workload_msg, zmq::recv_flags::none); // skip envelop delimiter
        //std::cout << "Worker " << worker_id << " received delimiter " << std::quoted(workload_msg.to_string()) << " [" << rc.value() << "]" << std::endl;
        //rc = worker.recv(workload_msg, zmq::recv_flags::none);
        //std::cout << "Worker " << worker_id << " received workload " << std::quoted(workload_msg.to_string()) << " [" << rc.value() << "]" << std::endl;
        auto workload = workload_msg.to_string();

        std::cout << "Worker " << worker_id << " received request: " << workload << std::endl;
        if (MSG_WORKER_TERM == workload)
        {
            g_terminate_pool = true;
        }
        else if (workload.find(MSG_JOB_REQ) != std::string::npos)
        {
            std::istringstream sin(workload);
            sin >> workload >> workload;
            if (sin)
            {
                add_job(workload);
            }
            else
            {
                std::cerr << "Worker " << worker_id << "failed to process workload: " << workload_msg << std::endl;
            }
        }

        //if (g_terminate_pool || converters.empty())
        if (g_terminate_pool)
        {
            std::cout << "Terminate pool thread. " << g_done_count << " jobs done." << std::endl;
            //std::unique_lock<std::mutex> lock(g_shutdown_mtx);
            ++g_shutdown_count;
            //g_cv_shutdown.notify_one();
            return 0;
        }

        if (converters.empty())
        {
            continue;
        }

        auto job = converters.front();
        converters.pop_front();
        //lock.unlock();

        std::cout << "Processing job for " << job.get_doc() << std::endl;
        if (!job.convert())
        {
            std::cerr << "Failed to convert " << job.get_doc() << ". Error=" << job.get_html_error_code() << std::endl;
        }
        else
        {
            const unsigned char* doc_buffer = nullptr;
            std::cout << "Converted document size " << job.get_output_buffer(&doc_buffer) << std::endl;
            ++g_done_count;
        }
    }

    return 1;
}

int main(int argc, char* argv[])
{
    vector<string> inputDocs;

    wkhtmltopdf_init(1);
    cout << "WKHtmlToX version " << wkhtmltopdf_version() << endl;

    zmq::context_t context(1);
    auto backend_port = 8877;

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
        else if (arg == "-worker")
        {
            //__debugbreak();
            //DebugBreak();
            g_is_worker = true;
        }
        else if (arg.find("-backend=") != std::string::npos)
        {
            std::istringstream sin {arg};
            char c;
            while (sin >> c && c != '=') {}
            sin >> backend_port;
            if (!sin)
            {
                std::cerr << "Argument -backend is malformed: " << sin.str() << std::endl;
                std::cerr << "port: " << backend_port << std::endl;
            }
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

        if (g_is_worker)
        {
            add_job(doc);
        }
    }
    cout << "Backend port: " << backend_port << endl;
    cout << endl;

    if (g_is_worker)
    {
        worker_main(context, backend_port);
    }
    else
    {
        // Distribute all given input documents to the workers
        //
        zmq::socket_t broker(context, zmq::socket_type::router);
        std::vector<std::vector<std::string>> doc_matrix(max_threads);
        //for (auto i = 0u; i < inputDocs.size(); ++i)
        //{
            //auto worker_idx = i % max_threads;
            //doc_matrix[worker_idx].push_back(inputDocs[i]);
        //}

        auto endpoint = std::string("tcp://*:") + std::to_string(backend_port);
        broker.bind(endpoint.data());

        for (auto i = 0u; i < max_threads; ++i)
        {
            g_workers.emplace_back(argv[0], backend_port, doc_matrix[i]);
        }

        for (auto& doc : inputDocs)
        {
            std::cout << "Deploy job for " << std::quoted(doc) << std::endl;

            //  Next message gives us least recently used worker
            zmq::message_t identity_msg;
            auto rc = broker.recv(identity_msg, zmq::recv_flags::none);
            if (!rc)
            {
                std::cerr << "Failed to receive worker's identity" << std::endl;
                break;
            }
            std::cout << "Received worker's identity " << std::quoted(identity_msg.to_string()) << std::endl;

            // Receive the workers response
            zmq::message_t reply_msg;
            rc = broker.recv(reply_msg, zmq::recv_flags::none); // skip envelop delimiter
            rc = broker.recv(reply_msg, zmq::recv_flags::none);
            if (!rc)
            {
                std::cerr << "Failed to receive worker's reply" << std::endl;
                break;
            }
            std::cout << "Received worker's reply " << std::quoted(reply_msg.to_string()) << std::endl;

            auto reply = reply_msg.to_string();
            if (MSG_WORKER_READY == reply)
            {
                ++g_done_count;
            }

            // Deploy more work
            std::ostringstream sout;
            sout << MSG_JOB_REQ << ' ' << doc;
            std::array<zmq::const_buffer, 3> req_msgs =
            {
                zmq::buffer(identity_msg.to_string()),
                zmq::str_buffer(""),     // envelop delimiter
                zmq::buffer(sout.str())  // workload
            };
            try
            {
                std::cout << "Sending multi-part JOB_REQ {" << identity_msg.to_string() << ", " << sout.str() << "}" << std::endl;
                if (!zmq::send_multipart(broker, req_msgs))
                {
                    std::cerr << "Failed to send JOB_REQ." << std::endl;
                }
            }
            catch (const zmq::error_t& err)
            {
                std::cerr << "Failed to send JOB_REQ." << err.what() << std::endl;
            }
        }

        shutdown_pool(broker);
    }

    //std::this_thread::sleep_for(2000ms);
    //if (wait_for_all_jobs())
    //{
        //std::cout << "All " << g_job_count << " jobs have finished" << std::endl;
    //}
    //else
    //{
        //std::cerr << "Timeout occurred while waiting for jobs to finish" << std::endl;
    //}

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


