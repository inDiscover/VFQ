// BillFormatter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <array>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;

//void func(char val[2][4])
void func(const char* val[])
{
    std::cout << "func:\n";
    std::cout << val[0] << std::endl;
    std::cout << val[1] << std::endl;
}

void func_modern(const array<const char*, 2>& val)
{
    std::cout << "func_modern:\n";
    std::cout << val[0] << std::endl;
    std::cout << val[1] << std::endl;
}

string* func_string(const vector<string>& val)
{
    //string other;
    string* str = new string("Good morning");
    //string str = "Good morning";
    *str = "Good morning, Dr. Chandra!";
    //std::cout << "String length: " << str->size() << std::endl;
    std::cout << "func_string:\n";
    std::cout << val[0] << std::endl;
    std::cout << val[1] << std::endl;
    //delete str;
    return str;
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

int main(int argc, char* argv[])
{
    vector<string> inputDocs;

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
    }
    cout << endl;

    /*
    argv[0]: "...exe"
    argv[1]: "/d"
    argv[2]: "*rc"

    
    "Hello World\0"
    [H][e][l][l][o][ ]....
     ^
     |
     char* pstr;

    "/d\0"
    {
    [/][d][\0],
    [*][r][c][\0]
    }

    char arr[5][10];
    [/][d][\0][][],[*][r][c][\0][],....


    */

    std::cout << "Hello World!\n";
    std::cout << "Arg Count: " << argc << std::endl;

    const char* val[] = 
    {
        "/d",
        "*rc"
    };
    char val1[2][8] = 
    {
        "/d",
        "*rc"
    };

    // C-style array of pointers
    const char* ptrs[2];
    ptrs[0] = val1[0];
    ptrs[1] = val1[1];
    ptrs[0]++;
    func(ptrs);

    // std::array of pointers
    func_modern({ val[0] + 1, val[1] });

    // Vector of std::strings
    auto* str = func_string({ val[0] + 1, val[1] });
    delete str;

}

// Topics:
// -------
// 
// - Review arrays
// - std::string
// - Command line parser
// - Operator overloading
//
// - struct
// - 

