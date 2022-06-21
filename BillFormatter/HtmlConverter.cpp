#include "HtmlConverter.h"

namespace
{
	/* Print out loading progress information */
	void progress_changed(wkhtmltopdf_converter* c, int p) {
		printf("%3d%%\r", p); // 100% , last % symbol is the escape character
		fflush(stdout); // might be to fill the buffer and once the buffer got filled then output send to the output stream
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

HtmlConverter::HtmlConverter(const std::string& in)
{
	auto out = in + ".pdf";
	globalSettings = wkhtmltopdf_create_global_settings();
    // out.data() => data is a method in a String class which returns character pointer so that there is no data typemismatch between String (out var defined in C++) and Char* defined in C
    // global settings (Structure) => settings related to the output file
	wkhtmltopdf_set_global_setting(globalSettings, "out", out.data());
	converter = wkhtmltopdf_create_converter(globalSettings);
    // object settings (Structure) => settings related to input file (In our case it is related to HTML files)
	settings = wkhtmltopdf_create_object_settings();
	wkhtmltopdf_set_object_setting(settings, "page", in.data());
    //At this time , converter knows what to convert
	wkhtmltopdf_add_object(converter, settings, nullptr);

	///* Call the progress_changed function when progress changes */
	//wkhtmltopdf_set_progress_changed_callback(converter, progress_changed);

	///* Call the phase _changed function when the phase changes */
	//wkhtmltopdf_set_phase_changed_callback(converter, phase_changed);

	///* Call the error function when an error occurs */
	//wkhtmltopdf_set_error_callback(converter, error);

	///* Call the warning function when a warning is issued */
	//wkhtmltopdf_set_warning_callback(converter, warning);
}

HtmlConverter::~HtmlConverter()
{
	wkhtmltopdf_destroy_converter(converter);
	wkhtmltopdf_destroy_object_settings(settings);
    wkhtmltopdf_destroy_global_settings(globalSettings);
}

bool HtmlConverter::Convert()
{
	return wkhtmltopdf_convert(converter);
}
