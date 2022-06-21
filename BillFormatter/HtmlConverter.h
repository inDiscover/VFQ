#pragma once
#include <string>

#include "wkhtmltox/pdf.h"

class HtmlConverter
{
public:
	HtmlConverter(const std::string& in);
	virtual ~HtmlConverter();
	bool Convert();

private:
	std::string inputDocument;
	wkhtmltopdf_global_settings* globalSettings = nullptr;
	wkhtmltopdf_converter* converter = nullptr;
	wkhtmltopdf_object_settings* settings = nullptr;
};

