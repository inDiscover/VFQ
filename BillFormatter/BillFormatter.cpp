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

//#include "mycomplex.h"
//class mycomplex;

#define _MAX_SIZE 123
constexpr int MAX_SIZE = 123;
#define MAX(lhs,rhs){lhs>rhs?lhs:rhs}

//typedef enum {
//    apple = 0,
//    orange,
//    banana
//} fruit_t;

using fruit = enum { apple, banana };

//typedef unsigned char byte_t;
using byte_t = unsigned char;

using namespace std;

//void func(char val[2][4])
//void func(const char* val[])
void func(const char* val[])
{
    int i = 123;
    int j = 456;
    int x = ( i > j ? i : j ), y = 987;
    std::cout << "func:\n";
    std::cout << val[0] << std::endl;
    std::cout << val[1] << std::endl;
}

void func(char* val[])
{
    int i = 123;
    int j = 456;
    int x = ( i > j ? i : j ), y = 987;
    std::cout << "func:\n";
    std::cout << val[0] << std::endl;
    std::cout << val[1] << std::endl;
    val[0][0] = 'A';
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

//template<typename T>
//auto get_value(T t)
//{
//    if constexpr (std::is_pointer_v<T>)
//        return *t; // deduces return type to int for T = int*
//    else
//        return t;  // deduces return type to int for T = int
//}

//ostream& bold(ostream& os) {
//    //return os << '\033' << '[';
//    return os << "<strong>";
//}

int main(int argc, char* argv[])
{
    //cout << "regular " << bold << "boldface" << endl;
    //cout << hex << 255 << " " << 254 << " " << hex << 253 << endl;
    //cout << "Pi: " << acos(-1.) << endl;
    //cout << "Pi: " << setprecision(3) << acos(-1.) << acos(-1.) << endl;
    //cout << "Pi: " << setw(8) << setprecision(3) << acos(-1.) << endl;
    //cout << "Pi: " << setfill('_') << setw(8) << setprecision(3) << acos(-1.) << endl;
    //cout << "Pi: " << setfill('_') << setprecision(3) << acos(-1.) << endl;
    //return 0;

    //mycomplex* c1 = new mycomplex;
    //cout << "Before: " << c1 << endl;
    //*c1 = 123;
    //cout << "After: " << c1 << endl;
    //delete c1;

    //mycomplex c2;
    //c2 = 23;

    //mycomplex<float> c1;
    //mycomplex<int> c0;
    //cin >> c1 >> c0;
    //cout << "The complex object is ";
    //cout << c1 << c0;

    //istringstream sin("67 89");
    //sin >> c1;
    //cout << "The complex object is ";
    //cout << c1;

    //return 0;

    //// if constexpr example usage
    //int i = 123;
    //int j = 456;
    //int* p = &j;
    //cout << "Value of int: " << get_value(i) << endl;
    //cout << "Value of pointer: " << get_value(p) << endl;
    //cout << endl;

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

    //std::cout << "Hello World!\n";
    //std::cout << "Arg Count: " << argc << std::endl;

    //const char* val[] = 
    //{
    //    "/d",
    //    "*rc"
    //};
    //char val1[2][8] = 
    //{
    //    "/d",
    //    "*rc"
    //};

    //// C-style array of pointers
    //char* ptrs[2];
    //ptrs[0] = val1[0];
    //ptrs[1] = val1[1];
    //ptrs[0]++;
    //cout << "val1: " << val1[0] << endl;
    //func(ptrs);
    //cout << "val1: " << val1[0] << endl;

    //// std::array of pointers
    //func_modern({ val[0] + 1, val[1] });

    //// Vector of std::strings
    //auto* str = func_string({ val[0] + 1, val[1] });
    //delete str;

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

// - <fstream>, <ofstream>: open, truncate
//   https://en.cppreference.com/w/cpp/io/basic_ofstream


