#pragma once
#include <string>
#include <vector>

#include "wkhtmltox/pdf.h"

using error_msgs_t = std::vector<std::string>;

class html_converter
{
public:
    html_converter() = default;
    html_converter(const html_converter& other);
    html_converter(html_converter&& other) noexcept;
    html_converter(const std::string& in, size_t bc);
    virtual ~html_converter();
    html_converter& operator=(const html_converter& other);
    html_converter& operator=(html_converter&& other) noexcept;
    bool convert();
    std::string get_doc() const;
    size_t get_bill_cycle() const;
    int get_html_error_code(error_msgs_t&) const;
    size_t get_output_buffer(const unsigned char** ppbuffer) const;

private:
    std::string input_document;
    size_t bill_cycle = 0;
    wkhtmltopdf_global_settings* global_settings = nullptr;
    wkhtmltopdf_converter* converter = nullptr;
    wkhtmltopdf_object_settings* settings = nullptr;
};

