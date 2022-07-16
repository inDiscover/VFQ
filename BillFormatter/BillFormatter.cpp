// BillFormatter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <fstream>
#include <ios>
#include <iostream>
#include <iomanip>
#include <array>
#include <iterator>
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
#include <optional>
#include <filesystem>
#include <cstdlib>

#if _OPENMP
#	include <omp.h>
#endif

#include "wkhtmltox/pdf.h"
#include "zmq.hpp"
#include "zmq_addon.hpp"

#include "HtmlConverter.h"
#include "worker.h"
#include "CmdServer.h"

using namespace std;
using namespace std::chrono_literals;

using out_file_name_t = std::optional<std::string>;
using job_success_result_t = std::optional<std::string>;

bool g_is_worker = false;
std::vector<worker> g_workers;
deque<html_converter> converters;
bool g_terminate_pool = false;
size_t g_done_count = 0u;
size_t g_job_count = 0u;
size_t g_shutdown_count = 0u;
size_t max_threads = 1;

static const std::string MSG_JOB_REQ = "JOB_REQ";
static const std::string MSG_WORKER_READY = "WORKER_READY";
static const std::string MSG_WORKER_TERM = "WORKER_TERM";

bool check_ready_result(const zmq::recv_result_t& ret, const std::vector<zmq::message_t>& recv_msgs)
{
    static const auto doc_msg_offset = 2u;
    static const auto error_msg_offset = 3u;
    auto success = true;
    if (error_msg_offset < ret.value_or(0u))
    {
        std::for_each(std::begin(recv_msgs)+error_msg_offset, std::end(recv_msgs), [](auto& el) {
                std::cerr << el.to_string() << std::endl;
                });
        success = false;
    }
    if (doc_msg_offset < ret.value_or(0u))
    {
        if (success)
        {
            std::cout << "Converted document: " << recv_msgs[doc_msg_offset].to_string() << std::endl;
        }
        else
        {
            std::cerr << "Failed to convert document: " << recv_msgs[doc_msg_offset].to_string() << std::endl;
        }
    }
    return success;
}

bool wait_for_shutdown(zmq::socket_t& broker)
{
    //std::unique_lock<std::mutex> lock(g_shutdown_mtx);
    //return g_cv_shutdown.wait_for(lock, TIMEOUT_SHUTDOWN_WORKERS, [] { return g_shutdown_count == max_threads; });

    //std::this_thread::sleep_for(5s);
    //return true;

    std::string identity;
    std::string delimiter;
    std::string workload;
    std::vector<zmq::message_t> recv_msgs;
    const auto ret = zmq::recv_multipart(broker, std::back_inserter(recv_msgs));

    //std::cout << "Broker got " << *ret << " messages" << std::endl;
    identity = recv_msgs.begin()->to_string();
    //delimiter = recv_msgs[1].to_string();
    workload = std::next(std::begin(recv_msgs))->to_string();
    //std::cout << "Identity: " << identity << " Delimiter: " << delimiter << " Workload: " << workload << std::endl;

    return MSG_WORKER_READY == workload;
}

void signal_shutdown_pool(zmq::socket_t& broker)
{
    // Signal shutdown of all workers
    for (auto i=0; i<max_threads; ++i)
    {
        //  Next message gives us least recently used worker
        std::string identity;
        std::string delimiter;
        std::string workload;
        std::vector<zmq::message_t> recv_msgs;
        const auto ret = zmq::recv_multipart(broker, std::back_inserter(recv_msgs));

        //std::cout << "Broker got " << *ret << " messages" << std::endl;
        identity = recv_msgs.begin()->to_string();
        //delimiter = recv_msgs[1].to_string();
        workload = std::next(std::begin(recv_msgs))->to_string();
        //std::cout << "Identity: " << identity << " Delimiter: " << delimiter << " Workload: " << workload << std::endl;

        if (check_ready_result(ret, recv_msgs))
        {
            ++g_done_count;
        }

        std::array<zmq::const_buffer, 3> req_msgs =
        {
            zmq::buffer(identity),
            zmq::str_buffer(""),          // envelop delimiter
            zmq::buffer(MSG_WORKER_TERM)  // shutdown request
        };
        try
        {
            if (!zmq::send_multipart(broker, req_msgs))
            {
                std::cerr << "Failed to send WORKER_TERM to worker " << identity << std::endl;
            }
            else
            {
                std::cout << "Sent WORKER_TERM to " << identity << std::endl;
            }
        }
        catch (const zmq::error_t& err)
        {
            std::cerr << "Failed to send WORKER_TERM to worker " << identity << ": " << err.what() << std::endl;
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

    if (!wait_for_shutdown(broker))
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

//bool wait_for_all_jobs()
//{
//    std::this_thread::sleep_for(6s);
//    return true;
//}

void signal_worker_ready(zmq::socket_t& worker, const std::string& worker_id, const std::string& job, const error_msgs_t& error_msgs)
{
    std::vector<zmq::const_buffer> req_msgs =
    {
        zmq::buffer(MSG_WORKER_READY)
    };
    req_msgs.emplace_back(zmq::buffer(job));
    std::for_each(error_msgs.begin(), error_msgs.end(), [&req_msgs](auto& el) { req_msgs.emplace_back(zmq::buffer(el)); });
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

void collect_inputs(const std::string& in, std::vector<std::string>& input_docs)
{
    using namespace std::filesystem;
    path in_path(in);

    if (is_directory(in_path))
    {
        for (auto& entry : directory_iterator(in_path))
        {
            auto entry_path = entry.path();
            if (entry.is_regular_file() && entry_path.extension() == ".html")
            {
                input_docs.push_back(entry_path.generic_string());
            }
        }
    }
    else if (is_regular_file(in_path))
    {
        input_docs.push_back(in_path.generic_string());
    }
    else if (in.find("file://") == 0 && in.find(".html") == in.size() - 5)
    {
        input_docs.push_back(in);
    }
    else
    {
        std::cerr << "Drop input: " << in << std::endl;
    }
}

out_file_name_t get_out_file_name(const std::string& in_path)
{
    std::string out_file_name;
    auto path = in_path;
    auto prefix_end = path.find_last_of("/");

    if (prefix_end == std::string::npos && 0 < path.size())
    {
        out_file_name = path;
    }
    else if (prefix_end != std::string::npos)
    {
        out_file_name = path.substr(prefix_end + 1);
    }

    return out_file_name.empty() ? std::nullopt : out_file_name_t(out_file_name);
}

void add_job(const std::string& doc)
{
    //std::cout << "Add job for " << doc << std::endl;

    {
        //std::unique_lock<std::mutex> lock(g_queue_mtx);
        converters.emplace_back(doc);
        ++g_job_count;
    }

    //g_cv_pool.notify_one();
}

job_success_result_t process_pending_jobs(const std::string& out_dir, error_msgs_t& error_msgs)
{
    std::string job_doc;
    while (!converters.empty())
    {
        auto job = converters.front();
        converters.pop_front();
        job_doc = job.get_doc();

        //std::cout << "Processing job for " << job.get_doc() << std::endl;
        if (!job.convert())
        {
            //auto ret = job.get_html_error_code(error_msgs);
            //std::string errors;
            //std::for_each(error_msgs.begin(), error_msgs.end(), [&errors](auto& el) { errors += el + '\n'; });
            //std::cerr << "Failed to convert " << job.get_doc() << ". Error code: " << ret << std::endl;
            //std::cerr << errors;

            return {};
        }
        else
        {
            const unsigned char* doc_buffer = nullptr;
            auto doc_size = job.get_output_buffer(&doc_buffer);
            //std::cout << "Converted document size " << doc_size << std::endl;
            if (0 < doc_size)
            {
                ++g_done_count;

                const auto out_file_name = get_out_file_name(job.get_doc());

                if (out_file_name)
                {
                    using namespace std;
                    auto out_path = out_dir + "/" + *out_file_name + ".pdf";
                    std::ofstream fout(out_path, ios::out|ios::trunc|ios::binary);
                    fout.write(reinterpret_cast<const char*>(doc_buffer), doc_size);
                    if (!fout)
                    {
                        //std::cerr << "Failed to write output file to: " << out_path << std::endl;
                        error_msgs.emplace_back(std::string("Error: Failed to write output file to: ") + out_path);
                        return {};
                    }
                }
            }
        }
    }
    return job_doc;
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

static unsigned int worker_main(zmq::context_t& context, int backend_port, const std::string& out_dir)
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

    error_msgs_t conversion_errors;

    // Process preloaded jobs first
    auto job_succeeded = process_pending_jobs(out_dir, conversion_errors);

    for (;;)
    {
        // Unlock this worker
        //std::unique_lock<std::mutex> lock(g_queue_mtx);
        //g_cv_pool.wait(lock, [] {
        //    return !converters.empty() || g_terminate_pool;
        //});

        //std::cout << "Worker " << worker_id << " signals WORKER_READY" << std::endl;
        signal_worker_ready(worker, worker_id, job_succeeded.value_or(""), conversion_errors);
        conversion_errors.clear();

        //std::cout << "Worker " << worker_id << " waiting for job requests..." << std::endl;

        std::vector<zmq::message_t> recv_msgs;
        const auto rc = zmq::recv_multipart(worker, std::back_inserter(recv_msgs));
        if (!rc)
        {
            std::cerr << "Worker " << worker_id << " failed to receive multi-part job request" << std::endl;
            break;
        }
        //std::cout << "Worker got " << *rc << " messages" << std::endl;
        auto workload = recv_msgs.rbegin()->to_string();

        //std::cout << "Worker " << worker_id << " received request: " << workload << std::endl;
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
                std::cerr << "Worker " << worker_id << "failed to process workload: " << workload << std::endl;
            }
        }

        //if (g_terminate_pool || converters.empty())
        if (g_terminate_pool)
        {
            std::cout << "Terminate pool thread. " << g_done_count << " jobs done." << std::endl;
            //std::unique_lock<std::mutex> lock(g_shutdown_mtx);
            ++g_shutdown_count;
            signal_worker_ready(worker, worker_id, job_succeeded.value_or(""), {});
            //
            //std::this_thread::sleep_for(2000ms);
            //g_cv_shutdown.notify_one();
            worker.set(zmq::sockopt::linger, 0);
            return 0;
        }

        if (converters.empty())
        {
            continue;
        }

        job_succeeded = process_pending_jobs(out_dir, conversion_errors);
    }

    worker.set(zmq::sockopt::linger, 0);
    return 1;
}

int main(int argc, char* argv[])
{
    vector<string> inputDocs;
    std::string out_dir = "out";
    std::optional<int> override_workers_count;

    wkhtmltopdf_init(1);
    cout << "WKHtmlToX version " << wkhtmltopdf_version() << endl;

    zmq::context_t context(1);
    auto backend_port = 8877;

    // Parsing the command line arguments and options
    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];

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
        else if (arg.find("-out=") != std::string::npos)
        {
            std::istringstream sin {arg};
            char c;
            while (sin >> c && c != '=') {}
            sin >> out_dir;
            if (!sin)
            {
                std::cerr << "Argument -out is malformed: " << sin.str() << std::endl;
                std::cerr << "out_dir: " << out_dir << std::endl;
            }
        }
        else if (arg.find("-workers=") != std::string::npos)
        {
            std::istringstream sin {arg};
            char c;
            int val = 0;
            while (sin >> c && c != '=') {}
            sin >> val;
            if (!sin)
            {
                std::cerr << "Argument -workers is malformed: " << sin.str() << std::endl;
                std::cerr << "override_workers_count: " << *override_workers_count << std::endl;
            }
            else
            {
                override_workers_count = make_optional<int>(val);
            }
        }
        else
        {
            collect_inputs(arg, inputDocs);
        }
    }

    max_threads = std::thread::hardware_concurrency();

#if _OPENMP
    max_threads = std::max(max_threads, omp_get_num_procs());
#endif
    max_threads = override_workers_count.value_or(max_threads);
    //max_threads = 1;

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
        worker_main(context, backend_port, out_dir);
    }
    else
    {
        // Distribute all given input documents to the workers
        //
        zmq::socket_t broker(context, zmq::socket_type::router);
        std::vector<std::vector<std::string>> doc_matrix(max_threads);

        //for (auto i = 0u; i < inputDocs.size(); ++i)
        //{
        //    auto worker_idx = i % max_threads;
        //    doc_matrix[worker_idx].push_back(inputDocs[i]);
        //}

        auto endpoint = std::string("tcp://*:") + std::to_string(backend_port);
        broker.bind(endpoint.data());

        CmdServer cmd_receiver(context);

        for (auto i = 0u; i < max_threads; ++i)
        {
            g_workers.emplace_back(argv[0], backend_port, out_dir, doc_matrix[i]);
        }

        // Process all initial conversion jobs and start listening to client's requests
        for (;;)
        {
			for (auto& doc : inputDocs)
			{
				//std::cout << "Deploy job for " << std::quoted(doc) << std::endl;

				//  Next message gives us least recently used worker
				std::string identity;
				std::string delimiter;
				std::string workload;
				std::vector<zmq::message_t> recv_msgs;
				const auto ret = zmq::recv_multipart(broker, std::back_inserter(recv_msgs));

				//std::cout << "Broker got " << *ret << " messages" << std::endl;
				identity = recv_msgs.begin()->to_string();
				//delimiter = recv_msgs[1].to_string();
				workload = std::next(std::begin(recv_msgs))->to_string();
				//std::cout << "Identity: " << identity << " Delimiter: " << delimiter << " Workload: " << workload << std::endl;

				if (check_ready_result(ret, recv_msgs))
				{
					++g_done_count;
				}

				if (MSG_WORKER_READY == workload)
				{
				}
				else
				{
					std::cerr << "Broker got unexpected message from worker " << identity << ": " << workload << std::endl;
				}

				// Deploy more work
				std::ostringstream sout;
				sout << MSG_JOB_REQ << ' ' << doc;
				workload = sout.str();
				std::array<zmq::const_buffer, 3> req_msgs =
				{
					zmq::buffer(identity),
					zmq::str_buffer(""),     // envelop delimiter
					zmq::buffer(workload)  // workload
				};
				try
				{
					std::cout << "Sending multi-part JOB_REQ {" << identity << ", " << sout.str() << "}" << std::endl;
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
		}

        //std::this_thread::sleep_for(5s);
        std::this_thread::sleep_for(1s);
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


