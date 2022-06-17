// BillFormatter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <iomanip>

#include "wkhtmltox/pdf.h"

using namespace std;

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

	wkhtmltopdf_global_settings *global_settings = wkhtmltopdf_create_global_settings();

    // Print the list of arguments
    cout << "Input documents:\n";
    for (auto doc : inputDocs)
    {
		cout << doc << endl;

		auto out = doc + ".pdf";
		wkhtmltopdf_set_global_setting(global_settings, "out", out.data());
		wkhtmltopdf_converter* converter = wkhtmltopdf_create_converter(global_settings);
		wkhtmltopdf_object_settings* settings = wkhtmltopdf_create_object_settings();
		wkhtmltopdf_set_object_setting(settings, "page", doc.data());
		wkhtmltopdf_add_object(converter, settings, nullptr);

		/* Call the progress_changed function when progress changes */
		wkhtmltopdf_set_progress_changed_callback(converter, progress_changed);

		/* Call the phase _changed function when the phase changes */
		wkhtmltopdf_set_phase_changed_callback(converter, phase_changed);

		/* Call the error function when an error occurs */
		wkhtmltopdf_set_error_callback(converter, error);

		/* Call the warning function when a warning is issued */
		wkhtmltopdf_set_warning_callback(converter, warning);

		if (!wkhtmltopdf_convert(converter))
		{
			cout << "Failed to convert " << doc << endl;
		}

		wkhtmltopdf_destroy_converter(converter);
		wkhtmltopdf_destroy_object_settings(settings);
    }
    cout << endl;

    wkhtmltopdf_destroy_global_settings(global_settings);
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


