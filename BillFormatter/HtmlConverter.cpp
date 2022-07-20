#include "HtmlConverter.h"
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

error_msgs_t error_msgs;

namespace
{
    /* Print out loading progress information */
    void progress_changed(wkhtmltopdf_converter* c, int p) {
        //printf("%3d%%\r", p); // 100% , last % symbol is the escape character
        //fflush(stdout); // might be to fill the buffer and once the buffer got filled then output send to the output stream
    }

    /* Print loading phase information */
    void phase_changed(wkhtmltopdf_converter* c) {
        //int phase = wkhtmltopdf_current_phase(c);
        //printf("%s\n", wkhtmltopdf_phase_description(c, phase));
    }

    /* Print a message to stderr when an error occurs */
    void error(wkhtmltopdf_converter* c, const char* msg) {
        //fprintf(stderr, "Error: %s\n", msg);
        error_msgs.emplace_back(std::string("Error: ") + msg);
    }

    /* Print a message to stderr when a warning is issued */
    void warning(wkhtmltopdf_converter* c, const char* msg) {
        //fprintf(stderr, "Warning: %s\n", msg);
        error_msgs.emplace_back(std::string("Warning: ") + msg);
    }
}

html_converter::html_converter()
{
}

html_converter::html_converter(const html_converter& other)
    : input_document(other.input_document)
    , global_settings(other.global_settings)
    , converter(other.converter)
    , settings(other.settings)
{
    const_cast<html_converter&>(other).input_document = "";
    const_cast<html_converter&>(other).global_settings = nullptr;
    const_cast<html_converter&>(other).converter = nullptr;
    const_cast<html_converter&>(other).settings = nullptr;
}

html_converter::html_converter(html_converter&& other) noexcept
    : input_document(std::exchange(other.input_document, ""))
    , global_settings(std::exchange(other.global_settings, nullptr))
    , converter(std::exchange(other.converter, nullptr))
    , settings(std::exchange(other.settings, nullptr))
{
}

html_converter::html_converter(const std::string& in)
{
    input_document = in;
}

html_converter::~html_converter()
{
    if (converter)
    {
        wkhtmltopdf_destroy_converter(converter);
        converter = nullptr;
    }
    //if (settings)
    //{
    //	wkhtmltopdf_destroy_object_settings(settings);
    //	settings = nullptr;
    //}
    if (global_settings)
    {
        wkhtmltopdf_destroy_global_settings(global_settings);
        global_settings = nullptr;
    }
}

html_converter& html_converter::operator=(const html_converter& other)
{
    return *this = html_converter(other);
}

html_converter& html_converter::operator=(html_converter&& other) noexcept
{
    std::swap(input_document, other.input_document);
    std::swap(converter, other.converter);
    std::swap(global_settings, other.global_settings);
    std::swap(settings, other.settings);
    return *this;
}

bool html_converter::convert()
{
    auto out = input_document + ".pdf";
    global_settings = wkhtmltopdf_create_global_settings();
    // out.data() => data is a method in a String class which returns character pointer so that there is no data typemismatch between String (out var defined in C++) and Char* defined in C
    // global settings (Structure) => settings related to the output file
    //wkhtmltopdf_set_global_setting(global_settings, "out", out.data());
    converter = wkhtmltopdf_create_converter(global_settings);
    // object settings (Structure) => settings related to input file (In our case it is related to HTML files)
    settings = wkhtmltopdf_create_object_settings();
    wkhtmltopdf_set_object_setting(settings, "page", input_document.data());
    //At this time , converter knows what to convert
    wkhtmltopdf_add_object(converter, settings, nullptr);

    /* call the progress_changed function when progress changes */
    wkhtmltopdf_set_progress_changed_callback(converter, progress_changed);

    /* call the phase _changed function when the phase changes */
    wkhtmltopdf_set_phase_changed_callback(converter, phase_changed);

    /* call the error function when an error occurs */
    wkhtmltopdf_set_error_callback(converter, error);

    /* call the warning function when a warning is issued */
    wkhtmltopdf_set_warning_callback(converter, warning);

    //std::cout << "Converting " << input_document << " @" << std::hex << std::this_thread::get_id() << std::dec << " ..." << std::endl;

    // Only for debugging
    //std::this_thread::sleep_for(500ms);
    //return true;
    // Only for debugging

    error_msgs.clear();
    return wkhtmltopdf_convert(converter);
}

std::string html_converter::get_doc() const
{
    return input_document;
} 

int html_converter::get_html_error_code(error_msgs_t& out_msgs) const
{
    if (converter)
    {
        out_msgs = error_msgs;
        return wkhtmltopdf_http_error_code(converter);
    }
    return 0;
} 

size_t html_converter::get_output_buffer(const unsigned char** ppbuffer) const
{
    if (converter)
    {
        return wkhtmltopdf_get_output(converter, ppbuffer);
    }
    return 0;
}
