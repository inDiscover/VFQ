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

#include "HtmlConverter.h"

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

    // Print the list of arguments
    cout << "Input documents:\n";
    for (auto doc : inputDocs)
    {
		cout << doc << endl;

        HtmlConverter converter(doc);
        if (!converter.Convert())
        {
            cout << "Failed to convert " << doc << endl;
        }
    }
    cout << endl;

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


