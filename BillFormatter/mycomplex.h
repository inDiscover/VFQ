#pragma once
#include <iostream>

using namespace std;

template<class T>
class mycomplex
{
private:
    T real, imag;

public:
    // Copy constructor
    mycomplex(const mycomplex& other)
    {
        real = other.real;
        imag = other.imag;
    }
    mycomplex(T r = 0, T i = 0)
    {
        real = r;   imag = i;
    }
    //void set(T r, T i)
    //{
    //    real = r; imag = i;
    //}
    friend ostream& operator << (ostream& out, const mycomplex& c);
    friend istream& operator >> (istream& in, mycomplex& c);
};

ostream& operator << (ostream& out, const mycomplex<int>& c)
{
    out << c.real;
    out << "+i" << c.imag << endl;
    return out;
}

ostream& operator << (ostream& out, const mycomplex<float>& c)
{
    out << c.real;
    out << "+i" << c.imag << endl;
    return out;
}

istream& operator >> (istream& in, mycomplex<int>& c)
{
    cout << "Enter Real Part ";
    in >> c.real;
    cout << "Enter Imaginary Part ";
    in >> c.imag;
    return in;
}

istream& operator >> (istream& in, mycomplex<float>& c)
{
    cout << "Enter Real Part ";
    in >> c.real;
    cout << "Enter Imaginary Part ";
    in >> c.imag;
    return in;
}
